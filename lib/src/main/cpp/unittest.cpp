#include <algorithm>
#include <iostream>
#include <iomanip>
#include <set>
#include <sstream>
#include <fstream>

#include <chrono>
#include <thread>
#include <mutex>

#include <regex>
#include <iterator>

#include <signal.h>
#include <setjmp.h>
#include <cstdlib>

#include <unistd.h>

#include "unittest.h"

using namespace CppUnitLite;

std::map<std::string, UnitTest::BoundedTest> *UnitTest::tests = nullptr;

long UnitTest::numSuccesses = 0L;
long UnitTest::numFailures = 0L;
long UnitTest::numErrors = 0L;
std::string UnitTest::currentTest;
bool UnitTest::expectToFail = false;
bool UnitTest::diagnosticMessagesBeforeResults = true;
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
		std::ostringstream out;
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
		std::ostringstream out;
		out << "at " << fileName << ":" << lineNumber
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
	using namespace std;

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
			std::string explanation = "Failed assertion: " + conditionStr
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
		tests = new std::map<std::string, UnitTest::BoundedTest>();
	}
	if (tests->count(functName) > 0) {
		std::cerr << "**Error: duplicate unit test named " << functName << std::endl;
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

int UnitTest::runTestGuarded (unsigned testNumber, std::string testName, TestFunction u,
		std::string& testExplanation)
{
	currentTest = testName;
	expectToFail = false;
	//UnitTest::msgRunning(testNumber, testName);
	try {
		signal(SIGFPE, &unitTestSignalHandler);
		signal(SIGSEGV, &unitTestSignalHandler);
		if (setjmp(unitTestSignalEnv)) {
			// Runtime error was caught
			std::ostringstream out;
			out << "# runtime error " << unitTestLastSignal;
			if (!expectToFail) {
				testExplanation =  UnitTest::msgFailed(testNumber, testName, out.str(), 0);
				return -1;
			} else {
				// OK (failed but was expected to fail)"
				UnitTest::msgXFailed(testNumber, testName, out.str(), 0);
			}
		} else {
			u();
			if (!expectToFail) {
				UnitTest::msgPassed(testNumber, testName, 0);
			} else {
				// Failed (passed but was expected to fail
				UnitTest::msgXPassed(testNumber, testName, 0);
				return 0;
			}
		}
		return 1;
	} catch (UnitTestFailure& ex) {
		if (!expectToFail) {
			testExplanation = UnitTest::msgFailed(testNumber, testName, ex.what(), 0);
			return 0;
		} else {
			// OK (failed but was expected to fail)"
			UnitTest::msgXFailed(testNumber, testName, ex.what(), 0);
			return 1;
		}
	} catch (std::exception& e) {
		if (!expectToFail) {
			UnitTest::msgError(testNumber, testName,
					"Unexpected error in " + currentTest + ": " +e.what(), 0);
			testExplanation = "";
			return -1;
		} else {
			// OK (exception but was expected to fail)"
			UnitTest::msgXFailed(testNumber, testName, "", 0);
			return 1;
		}
	} catch (...) {
		if (!expectToFail) {
			UnitTest::msgError(testNumber, testName, "Unexpected error in " + currentTest, 0);
			testExplanation = "";
			return -1;
		} else {
			// OK (exception but was expected to fail)"
			UnitTest::msgXFailed(testNumber, testName, "", 0);
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
void UnitTest::runTestUntimed (unsigned testNumber, std::string testName, TestFunction u)
{
	int testResult; // 1== passed, 0 == failed, -1 == erro
	std::string testExplanation;

	// No time-out supported if compiler does not have thread support.
	testResult = runTestGuarded (testNumber, testName, u, testExplanation);

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
		UnitTest::msg(std::string("# Test ") + currentTest + " failed due to "
				+ e.what() + "\n");
	}


}


#ifndef __MINGW32__

// Run a single unit test function.
void UnitTest::runTest (unsigned testNumber, std::string testName, TestFunction u, long timeLimit)
{
	if (timeLimit > 0L && !debuggerIsRunning())
	{
		int testResult = -99; // 1== passed, 0 == failed, -1 == error
		std::string testExplanation;

		std::mutex m;
		std::chrono::duration<int,std::milli> limit (timeLimit);
		std::chrono::duration<int,std::milli> incr (100);
		std::chrono::duration<int,std::milli> elapsed (0);

		std::thread t([&m, &testNumber, &testName, &u, &testResult, &testExplanation](){
			{
				int result = runTestGuarded (testNumber, testName, u, testExplanation);
				std::unique_lock<std::mutex> l2(m);
				testResult = result;
			}
		});
		t.detach();

		bool finished = false;
		do {
		    {
		    	std::unique_lock<std::mutex> l(m);
		    	finished = (testResult >= -1 || elapsed >= limit);
		    	elapsed += incr;
		    }
		    std::this_thread::sleep_for( incr );
		} while (!finished);

		if (testResult < -1) {
			++numFailures;
			failedTests.push_back(testName);
			std::ostringstream out;
			out << "# Test " << testNumber << " - " << currentTest << " still running after "
					<< timeLimit
					<< " milliseconds - possible infinite loop?";
			if (!expectToFail)
			{
				UnitTest::msg (
						UnitTest::msgFailed(testNumber, testName, out.str(), timeLimit)
				);
			}
			else
			{
				UnitTest::msgXFailed(testNumber, testName, out.str(), timeLimit);
				++numSuccesses;
				--numFailures;
			}
		}
		// Normal exit
		else if (testResult == 1) {
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
	}
	else
	{
		runTestUntimed (testNumber, testName, u);
	}

}

#else

// Run a single unit test function.
void UnitTest::runTest (unsigned testNumber, std::string testName, TestFunction u, long int timeLimit)
{
	runTestUntimed (testNumber, testName, u);
}

#endif


// Run all units tests whose name contains testNames[i],
// 0 <= i <= nTests
//
// Special case: If nTests == 0, runs all unit Tests.
void UnitTest::runTests (int nTests, char** testNames, char* program)
{
	std::set<std::string> testsToRun;
	// Check for GTest emulation
	for (int i = 0; i < nTests; ++i)
	{
		std::string arg = testNames[i];
	}

	std::string badTestSpecifications = "";

		for (int i = 0; i < nTests; ++i)
		{
			std::string testID = testNames[i];
			bool found = false;
			for (const auto& utest: *tests) {
				if (utest.first.find(testID) != std::string::npos) {
					testsToRun.insert(utest.first);
					found = true;
				}
			}
			if (!found)
			{
				for (const auto& utest: *tests) {
					const std::string& utestName = utest.first;
					std::string reducedName (1, utestName[0]);
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
				badTestSpecifications += "# Warning: No matching test found for input specification "
						+ testID + "\n";
			}

	}
	if (testsToRun.size() == 0) {
		for (const auto& utest: *tests) {
			testsToRun.insert(utest.first);
		}
	}

	// Emit TAP plan line
	UnitTest::msg ("1.." + std::to_string(testsToRun.size()));
	UnitTest::msg (badTestSpecifications);

	unsigned testNumber = 1;
	for (std::string testName: testsToRun) {
		BoundedTest test = (*tests)[testName];
		runTest (testNumber, testName, test.unitTest, test.timeLimit);
		++testNumber;
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


void UnitTest::msgRunning (unsigned testNumber, std::string testName)
{
	using namespace std;
	cout << "# starting " << testNumber << " - " << testName << endl;;
}

void UnitTest::msgPassed (unsigned testNumber, std::string testName, unsigned timeMS)
{
	using namespace std;
	cout << "ok " << testNumber << " - " << testName << endl;
}

void UnitTest::msgXPassed (unsigned testNumber, std::string testName, unsigned timeMS)
{
	UnitTest::msg(
			UnitTest::msgFailed(testNumber, testName,
					std::string("Test ") + std::to_string(testNumber) + " - " + testName
					+ " passed but was expected to fail.", timeMS)
	);
}



std::string UnitTest::msgFailed (unsigned testNumber, std::string testName,
		std::string diagnostics, unsigned timeMS)
{
	using namespace std;

	string diagnosticString = msgComment(diagnostics);
	string resultMsg = "not ok " + to_string(testNumber) + " - " + testName;

	if (diagnosticMessagesBeforeResults)
		return diagnosticString + "\n" + resultMsg;
	else
		return resultMsg + "\n" + diagnosticString;
}

std::string UnitTest::msgComment (const std::string& commentary) {
	const static std::string commentPrefix = "# ";
	std::string result;
	std::string startOfLine = commentary.substr(0, commentPrefix.size());
	if (startOfLine == commentPrefix)
		result = commentary;
	else
		result = commentPrefix + commentary;
	std::string::size_type pos = result.find('\n');
	while (pos != std::string::npos)
	{
		if (result.size() >= pos+1+commentPrefix.size())
		{
			startOfLine = result.substr(pos+1, commentPrefix.size());
			if (startOfLine != commentPrefix)
			{
				result.insert(pos+1, commentPrefix);
			}
		}
		else
		{
			result.insert(pos+1, commentPrefix);
		}
		pos = result.find('\n', pos+1);
	}
	return result;
}


void UnitTest::msgXFailed (unsigned testNumber, std::string testName, std::string diagnostics, unsigned timeMS)
{
	std::string diagnosticMsg = msgComment(std::string("Test ") + std::to_string(testNumber) + " failed but was expected to fail.");
	if (diagnosticMessagesBeforeResults)
		UnitTest::msg(diagnosticMsg);
	UnitTest::msgPassed(testNumber, testName, timeMS);
	if (!diagnosticMessagesBeforeResults)
		UnitTest::msg(diagnosticMsg);
}

void UnitTest::msgError (unsigned testNumber, std::string testName, std::string diagnostics, unsigned timeMS)
{
	std::string diagnosticMsg = msgComment("ERROR - " + diagnostics);
	if (diagnosticMessagesBeforeResults)
		UnitTest::msg(diagnosticMsg);
	UnitTest::msg("not ok " + std::to_string(testNumber) + " - " + testName);
	if (!diagnosticMessagesBeforeResults)
		UnitTest::msg(diagnosticMsg);
}

void UnitTest::msgSummary ()
{
	using namespace std;
	cout << "# UnitTest: passed " << numSuccesses << " out of "
		 << getNumTests() << " tests, for a success rate of "
		 << std::showpoint << std::fixed << std::setprecision(1)
		 << (100.0 * numSuccesses)/(float)getNumTests()
		 << "%" << endl;
}





void UnitTest::msg (const std::string& detailMessage)
{
	using std::cout;

	cout << detailMessage;
	if (detailMessage.size() > 0 &&
			detailMessage[detailMessage.size()-1] != '\n')
		cout << "\n";
	cout << std::flush;
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
	UnitTest::diagnosticMessagesBeforeResults = true;
	UnitTest::runTests(argc-1, argv+1, argv[0]);

	UnitTest::report();

	return 0;
}

#endif




