#include <algorithm>
#include <iostream>
#include <iomanip>
#include <set>
#include <sstream>
#include <fstream>

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <signal.h>
#include <setjmp.h>
#include <cstdlib>

#include <unistd.h>

#include "unittest.h"

using namespace std;
using namespace CppUnitLite;

std::map<std::string, UnitTest::BoundedTest> *UnitTest::tests = nullptr;

bool UnitTest::gTestMode = false;
long UnitTest::numSuccesses = 0L;
long UnitTest::numFailures = 0L;
long UnitTest::numErrors = 0L;
string UnitTest::currentTest;
bool UnitTest::expectToFail = false;
std::vector<std::string> UnitTest::callLog;
std::vector<std::string> UnitTest::failedTests;

#ifdef __amd64__
#define breakDebugger { asm volatile ("int $3"); }
#else
#define breakDebugger { }
#endif

template <>
std::string CppUnitLite::getStringRepr(std::string t)
{
	return std::string("\"") + t + '"';
}
template <>
std::string CppUnitLite::getStringRepr(const char t[])
{
	return CppUnitLite::getStringRepr(std::string(t));
}
template <>
std::string CppUnitLite::getStringRepr(char t)
{
	return std::string("'") + t + "'";
}
template <>
std::string CppUnitLite::getStringRepr(bool b)
{
	return (b) ? "true" : "false";
}


UnitTest::UnitTestFailure::UnitTestFailure (
		const char* conditionStr,
		const char* fileName, int lineNumber)
{
	if (!UnitTest::expectToFail) {
		ostringstream out;
		out << "Failed assertion " << conditionStr
				<< " in " << currentTest
				<< " at " << fileName << ", line "
				<< lineNumber << "\n";
		explanation = out.str();
	} else {
		explanation = "(expected to fail)";
	}
}

UnitTest::UnitTestFailure::UnitTestFailure (
		const std::string& conditionStr,
		const char* fileName, int lineNumber)
{
	if (!UnitTest::expectToFail) {
		ostringstream out;
		out << fileName << ":" << lineNumber << ": Failure"
				<< "\n\t" << conditionStr << "\n";
		explanation = out.str();
	} else {
		explanation = "(expected to fail)";
	}
}

const char* UnitTest::UnitTestFailure::what() const noexcept {
	return explanation.c_str();
}


AssertionResult::AssertionResult (bool theResult, std::string pexplain, std::string fexplain)
 : result(theResult), passExplanation(pexplain), failExplanation(fexplain)
{}



bool UnitTest::debuggerIsRunning()
{
     static bool debuggerDetected = false;
     const string traceField = "tracerpid";

     int pid = ::getpid();
     string statusFile = string("/proc/") + std::to_string(pid) + "/status";
     ifstream status (statusFile);
     if (status) {
    	 string line;
    	 getline (status, line);
    	 while (status) {
    		 transform(line.begin(), line.end(), line.begin(), ::tolower);
    		 if (line.find(traceField) != string::npos) {
    			 string::size_type k = line.find_first_of(" \t");
    			 if (k != string::npos) {
    				 line = line.substr(k+1);
    				 istringstream lineIn (line);
    				 int pid = -1;
    				 lineIn >> pid;
    				 if (pid > 0) {
    					 debuggerDetected = true;
    				 }
    			 }
    			 break;
    		 }
    		 getline (status, line);
    	 }
     }
     if (debuggerDetected)
     {
    	 UnitTest::msg("*Debugger detected -- test time limits will be ignored.\n");
     }
     return debuggerDetected;
}


void UnitTest::checkTest (AssertionResult assertionResult, std::string conditionStr,
		const char* fileName, int lineNumber)
{
	if (!assertionResult.result)
	{
		if (debuggerIsRunning())
		{
			string explanation = "Failed assertion: " + conditionStr
					+ "\n" + assertionResult.failExplanation;
			breakDebugger;
			// Unit test has failed.
			// Examine explanation for information
			explanation = explanation + " ";
		}
		if (assertionResult.failExplanation.size() > 0)
		{
			conditionStr += "\n\t" + assertionResult.failExplanation;
		}
		throw UnitTestFailure(conditionStr, fileName, lineNumber);
	}
}




//void UnitTest::checkTest (bool condition, const string& conditionStr,
//		const char* fileName, int lineNumber)
//{
//	checkTest(AssertionResult(condition), conditionStr.c_str(), fileName, lineNumber);
//}



// Print a simple summary report
void UnitTest::report ()
{
	UnitTest::msgSummary();
}


// Register a new UnitTest
int UnitTest::registerUT (std::string functName, int timeLimit, TestFunction funct)
{
	if (tests == nullptr)
	{
		tests = new map<std::string, UnitTest::BoundedTest>();
	}
	if (tests->count(functName) > 0) {
		cerr << "**Error: duplicate unit test named " << functName << endl;
	}
	(*tests)[functName] = BoundedTest(timeLimit, funct);
	return 0;
}

jmp_buf unitTestSignalEnv;
int unitTestLastSignal = 0;

void unitTestSignalHandler(int sig) {
	unitTestLastSignal = sig;
	longjmp (unitTestSignalEnv, sig);
}

int UnitTest::runTestGuarded (std::string testName, TestFunction u,
		std::string& testExplanation)
{
	currentTest = testName;
	expectToFail = false;
	UnitTest::msgRunning(testName);
	try {
		signal(SIGFPE, &unitTestSignalHandler);
		signal(SIGSEGV, &unitTestSignalHandler);
		if (setjmp(unitTestSignalEnv)) {
			// Runtime error was caught
			if (!expectToFail) {
				ostringstream out;
				out << "runtime error " << unitTestLastSignal;
				testExplanation =  out.str() + "\n"
						+ UnitTest::msgFailed(testName, 0);
				return -1;
			} else {
				// OK (failed but was expected to fail)"
				UnitTest::msgXFailed(testName, 0);
			}
		} else {
			u();
			if (!expectToFail) {
				UnitTest::msgPassed(testName, 0);
			} else {
				// Failed (passed but was expected to fail
				UnitTest::msgXPassed(testName, 0);
				return 0;
			}
		}
		return 1;
	} catch (UnitTestFailure& ex) {
		if (!expectToFail) {
			testExplanation = ex.what() + std::string("\n")
					+ UnitTest::msgFailed(testName, 0);
			return 0;
		} else {
			// OK (failed but was expected to fail)"
			UnitTest::msgXFailed(testName, 0);
			return 1;
		}
	} catch (exception& e) {
		if (!expectToFail) {
			UnitTest::msgError(testName, 0);
			testExplanation = "Unexpected error in " + currentTest
					+ ": " + e.what();
			return -1;
		} else {
			// OK (exception but was expected to fail)"
			UnitTest::msgXFailed(testName, 0);
			return 1;
		}
	} catch (...) {
		if (!expectToFail) {
			UnitTest::msgError(testName, 0);
			testExplanation = "Unexpected error in " + currentTest;
			return -1;
		} else {
			// OK (exception but was expected to fail)"
			UnitTest::msgXFailed(testName, 0);
			return 1;
		}
	}
}

/**
 * Reverses the expectation for the current test.  A test that fails or halts
 * with an error will be reported and counted as OK.  If that test succeeds,
 * it will be reported and counted as an error.
 *
 * Must be called before any assertions.
 */
void UnitTest::expectedToFail()
{
	expectToFail = true;
}

// Run a single unit test function with no timer.
void UnitTest::runTestUntimed (std::string testName, TestFunction u)
{
	int testResult; // 1== passed, 0 == failed, -1 == erro
	string testExplanation;

	// No time-out supported if compiler does not have thread support.
	testResult = runTestGuarded (testName, u, testExplanation);

	try {
		// Normal exit
		if (testResult == 1) {
			++numSuccesses;
		} else if (testResult == 0) {
			++numFailures;
			failedTests.push_back(testName);
			UnitTest::msg(testExplanation);
		} else if (testResult == -1) {
			++numErrors;
			failedTests.push_back(testName);
			UnitTest::msg(testExplanation);
		}
	} catch (std::runtime_error& e) {
		++numErrors;
		failedTests.push_back(testName);
		UnitTest::msg(string("Test ") + currentTest + " failed due to "
				+ e.what() + "\n");
	}


}


#ifndef __MINGW32__

// Run a single unit test function.
void UnitTest::runTest (std::string testName, TestFunction u, long timeLimit)
{
	if (timeLimit > 0L && !debuggerIsRunning())
	{
		int testResult; // 1== passed, 0 == failed, -1 == erro
		string testExplanation;

		std::mutex m;
		std::condition_variable cv;
		chrono::duration<int,std::milli> limit (timeLimit);

		std::thread t([&m, &cv, &testName, &u, &testResult, &testExplanation](){
			testResult = runTestGuarded (testName, u, testExplanation);
			cv.notify_one();
		});
		t.detach();

		try {
			std::unique_lock<std::mutex> l(m);
			if(cv.wait_for(l, limit) == std::cv_status::timeout)
				throw std::runtime_error("Timeout");

			// Normal exit
			if (testResult == 1) {
				++numSuccesses;
			} else if (testResult == 0) {
				++numFailures;
				failedTests.push_back(testName);
				UnitTest::msg(testExplanation);
			} else if (testResult == -1) {
				++numErrors;
				failedTests.push_back(testName);
				UnitTest::msg(testExplanation);
			}
		} catch (std::runtime_error& e) {
			++numFailures;
			failedTests.push_back(testName);
			ostringstream out;
			out << "Test " << currentTest << " still running after "
					<< timeLimit
					<< " milliseconds - possible infinite loop?\n";
			UnitTest::msg(out.str() + "\n" +
					UnitTest::msgFailed(testName, timeLimit));
		}
	}
	else
	{
		runTestUntimed (testName, u);
	}

}

#else

// Run a single unit test function.
void UnitTest::runTest (std::string testName, TestFunction u, long int timeLimit)
{
	runTestUntimed (testName, u);
}

#endif


// Run all units tests whose name contains testNames[i],
// 0 <= i <= nTests
//
// Special case: If nTests == 0, runs all unit Tests.
void UnitTest::runTests (int nTests, char** testNames, char* program)
{
	set<string> testsToRun;
	gTestMode = false;
	// Check for GTest emulation
	for (int i = 0; i < nTests; ++i)
	{
		string arg = testNames[i];
		static const string GTEST = "--gtest_";
		if (arg.length() > GTEST.length()
				&& arg.substr(0, GTEST.length()) == GTEST)
		{
			gTestMode = true;
			static const string GTESTFILTER = "--gtest_filter=";
			if (arg.length() > GTESTFILTER.length()
					&& arg.substr(0, GTESTFILTER.length()) == GTESTFILTER)
			{
				string filters = arg.substr(GTESTFILTER.length());
				string::size_type startPos = 0;
				string::size_type endPos = filters.find(":", startPos);
				while (endPos != string::npos && endPos < filters.length())
				{
					string testName = filters.substr(startPos, endPos-startPos);
					if (testName.length() > 0)
					{
						unsigned pos = testName.find(".");
						if (pos != string::npos)
						{
							testName = testName.substr(pos+1);
						}
					}
					if (testName.length() > 0 && tests->find(testName) != tests->end())
					{
						testsToRun.insert(testName);
					}
					startPos = endPos+1;
					endPos = filters.find(":", startPos);
				}
				string testName = filters.substr(startPos);
				if (testName.length() > 0)
				{
					unsigned pos = testName.find(".");
					if (pos != string::npos)
					{
						testName = testName.substr(pos+1);
					}
				}
				if (testName.length() > 0 && tests->find(testName) != tests->end())
				{
					testsToRun.insert(testName);
				}
			}
		}
	}
	if (!gTestMode)
	{
		for (int i = 0; i < nTests; ++i)
		{
			string testID = testNames[i];
			bool found = false;
			for (const auto& utest: *tests) {
				if (utest.first.find(testID) != string::npos) {
					testsToRun.insert(utest.first);
					found = true;
				}
			}
			if (!found)
			{
				for (const auto& utest: *tests) {
					const string& utestName = utest.first;
					string reducedName (1, utestName[0]);
					for (unsigned i = 1; i < utest.first.size(); ++i)
					{
						if (utestName[i] >= 'A' && utestName[i] <= 'Z')
						{
							reducedName += utestName[i];
						}
					}
					if (testID == reducedName)
					{
						testsToRun.insert(utest.first);
						found = true;
					}
				}
			}
			if (!found)
			{
				cerr << "*Warning: No matching test found for input specification "
						<< testID << endl;
			}
		}
	}
	if (testsToRun.size() == 0) {
		for (const auto& utest: *tests) {
			testsToRun.insert(utest.first);
		}
	}
	UnitTest::msg(string("Running main() from ") + program);
	UnitTest::msgStarting(testsToRun.size());
	for (string testName: testsToRun) {
		BoundedTest test = (*tests)[testName];
		runTest (testName, test.unitTest, test.timeLimit);
	}
}


/**
 * Clear the call log.
 */
void UnitTest::clearCallLog()
{
	callLog.clear();
}

/**
 * Position of oldest logged call.
 */
UnitTest::iterator UnitTest::begin()
{
	return callLog.begin();
}

/**
 * Position just after the most recently logged call.
 */
UnitTest::iterator UnitTest::end()
{
	return callLog.end();
}



/**
 * Log a call to a zero-parameter function.
 *
 * @param functionName name of the function
 */
void UnitTest::logCall (const std::string& functionName)
{
	callLog.push_back (functionName);
}


void UnitTest::msgStarting (unsigned nTests)
{
	if (gTestMode)
	{
		cout << "[==========] Running " << nTests << " tests from 1 test case.\n";
		cout << "[----------] Global test environment set-up.\n";
		cout << "[----------] " << nTests << " tests from Test" << endl;
	}
	else
		cout << "Running " << nTests << " tests." << endl;
}

void UnitTest::msgRunning (std::string testName)
{
	if (gTestMode)
	{
		cout << "[ RUN      ] Test." << testName << endl;
	}
	else
		cout << testName << ": " << flush;
}

void UnitTest::msgPassed (std::string testName, unsigned timeMS)
{
	if (gTestMode)
	{
		cout << "[       OK ] Test." << testName
				<< " (" << timeMS << " ms)" << endl;
	}
	else
		cout << "OK" << endl;
}

void UnitTest::msgXPassed (std::string testName, unsigned timeMS)
{
	UnitTest::msg("Test passed but was expected to fail.\n"
			+ UnitTest::msgFailed(testName, timeMS));
}

std::string UnitTest::msgFailed (std::string testName, unsigned timeMS)
{
	if (gTestMode)
	{
		return "[  FAILED  ] Test." + testName
				+ " (" + to_string(timeMS) +  " ms)";
	}
	else
		return "FAILED";
}
void UnitTest::msgXFailed (std::string testName, unsigned timeMS)
{
	UnitTest::msgPassed(testName, timeMS);
	UnitTest::msg("Test failed but was expected to fail.");
}

void UnitTest::msgError (std::string testName, unsigned timeMS)
{
	if (gTestMode)
	{
		cout << "[  FAILED  ] Test." << testName
				<< " ( " << timeMS << "ms)" << endl;
	}
	else
		cout << "ERROR" << endl;
}

void UnitTest::msgSummary ()
{
	if (gTestMode)
	{
		cout << "[----------] " << numErrors+numFailures+numSuccesses
				<< " tests from Test (0 ms total)" << endl;
		cout << "[==========] " << getNumTests()
				<< " tests from 1 test case ran.(1 ms total)\n";
		cout << "[  PASSED  ] " << numSuccesses <<
				((numSuccesses == 1) ? " test." : " tests.") << "\n";
		int numFailed = numFailures + numErrors;
		cout << "[  FAILED  ] " << numFailed <<
				((numFailed == 1) ? " test" : " tests");
		if (numFailed > 0)
			cout << ", listed below:" << endl;
		else
			cout << "." << endl;
		for (string failedTest: failedTests)
		{
			cout << "[  FAILED  ] Test." << failedTest << endl;
		}
		cout << "\n";
		cout << numFailed << " FAILED TEST";
		if (numFailed != 1)
			cout << "S";
		cout << endl;
	}
	else
	{
		cout << "UnitTest: passed " << numSuccesses << " out of "
				<< getNumTests() << " tests, for a success rate of "
				<< std::showpoint << std::fixed << std::setprecision(1)
		<< (100.0 * numSuccesses)/(float)getNumTests()
		<< "%" << endl;
	}
}





void UnitTest::msg (const std::string& detailMessage)
{
	cout << detailMessage;
	if (detailMessage.size() > 0 &&
			detailMessage[detailMessage.size()-1] != '\n')
		cout << "\n";
	cout << flush;
}











StringContainsMatcher::StringContainsMatcher (const std::string& t): right(t) {}
AssertionResult StringContainsMatcher::eval(const std::string& e) const {
	auto result = e.find(right);
	return AssertionResult( result != std::string::npos,
			"Found " + getStringRepr(right) + " starting in position "
				+ getStringRepr(result) + " of " + getStringRepr(e),
			"Within " + getStringRepr(e) + ", cannot find " + getStringRepr(right));
}

CppUnitLite::StringContainsMatcher
contains(const char* t)
{
	return CppUnitLite::StringContainsMatcher(std::string(t));
}

CppUnitLite::StringContainsMatcher
contains(const std::string& t)
{
	return CppUnitLite::StringContainsMatcher(t);
}

StringEndsWithMatcher::StringEndsWithMatcher (const std::string& t): right(t) {}
AssertionResult StringEndsWithMatcher::eval(const std::string& e) const {
	std::string eStr = getStringRepr(e);
	std::string rightStr = getStringRepr(right);
	bool result = (right.size() <= e.size())
			&& equal(right.begin(), right.end(),
				e.begin() + e.size() - right.size());

	return AssertionResult(result,
			eStr + " ends with " + rightStr,
			eStr + " does not end with " + rightStr);
}

StringEndsWithMatcher
endsWith(const char* t)
{
	return StringEndsWithMatcher(std::string(t));
}

StringEndsWithMatcher
endsWith(const std::string& t)
{
	return StringEndsWithMatcher(t);
}

StringBeginsWithMatcher::StringBeginsWithMatcher (const std::string& t)
: right(t) {}

AssertionResult StringBeginsWithMatcher::eval(const std::string& e) const
{
	std::string eStr = getStringRepr(e);
	std::string rightStr = getStringRepr(right);
	bool result = (right.size() <= e.size())
			&& equal(right.begin(), right.end(), e.begin());
	return AssertionResult(result,
			eStr + " begins with " + rightStr,
			eStr + " does not begin with " + rightStr
			);
}

StringBeginsWithMatcher beginsWith(const char* t)
{
	return CppUnitLite::StringBeginsWithMatcher(std::string(t));
}

StringBeginsWithMatcher beginsWith(const std::string& t)
{
	return StringBeginsWithMatcher(t);
}

StringBeginsWithMatcher startsWith(const char* t)
{
	return StringBeginsWithMatcher(std::string(t));
}

StringBeginsWithMatcher startsWith(const std::string& t)
{
	return StringBeginsWithMatcher(t);
}


AssertionResult NullMatcher::eval(const void* p) const
{
	return AssertionResult(p == nullptr, "", "");
}

CppUnitLite::NullMatcher isNull()
{
	return CppUnitLite::NullMatcher();
}

AssertionResult NotNullMatcher::eval(const void* p) const {
	return AssertionResult(p != nullptr, "", "");
}

CppUnitLite::NotNullMatcher isNotNull()
{
	return CppUnitLite::NotNullMatcher();
}





#ifndef NOMAIN

int main(int argc, char** argv)
{
	UnitTest::runTests(argc-1, argv+1, argv[0]);

	UnitTest::report();

	return 0;
}

#endif




