#include <iostream>
#include <iomanip>
#include <set>
#include <sstream>

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <signal.h>
#include <setjmp.h>
#include <cstdlib>

#include <sys/ptrace.h>


#include "unittest.h"

using namespace std;
using namespace CppUnitLite;

std::map<std::string, UnitTest::BoundedTest> *UnitTest::tests = nullptr;

long UnitTest::numSuccesses = 0L;
long UnitTest::numFailures = 0L;
long UnitTest::numErrors = 0L;
string UnitTest::currentTest;
bool UnitTest::expectToFail = false;
std::vector<std::string> UnitTest::callLog;

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
		out << "Failed assertion in "
				<< currentTest
				<< " at " << fileName << ", line "
				<< lineNumber
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
     static bool firstTime = true;
     static bool debuggerDetected = false;
#ifndef __MINGW32__
     if (firstTime)
     {
    	 firstTime = false;
    	 if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0)
    		 debuggerDetected = true;
    	 else
    		 ptrace(PTRACE_DETACH, 0, 1, 0);
    	 if (debuggerDetected)
    	 {
    		 cerr << "*Debugger detected -- test time limits will be ignored."
    				 << endl;
    	 }
     }
#endif
     return debuggerDetected;
}


void UnitTest::checkTest (AssertionResult assertionResult, std::string conditionStr,
		const char* fileName, int lineNumber)
{
	if (!assertionResult.result)
	{
		if (debuggerIsRunning()) breakDebugger;  // Unit test has failed.
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
void UnitTest::report (std::ostream& out)
{
	out << "UnitTest: passed " << numSuccesses << " out of "
			<< getNumTests() << " tests, for a success rate of "
			<< std::showpoint << std::fixed << std::setprecision(1)
	<< (100.0 * numSuccesses)/(float)getNumTests()
	<< "%" << endl;
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
	cerr << testName << ": " << flush;
	try {
		signal(SIGFPE, &unitTestSignalHandler);
		signal(SIGSEGV, &unitTestSignalHandler);
		if (setjmp(unitTestSignalEnv)) {
			// Runtime error was caught
			if (!expectToFail) {
			cerr << "failed" << endl;
			ostringstream out;
			out << "runtime error " << unitTestLastSignal;
			testExplanation =  out.str();
			return -1;
			} else {
				cerr << "OK (failed but was expected to fail)" << endl;
			}
		} else {
			u();
			if (!expectToFail) {
				cerr << "OK" << endl;
			} else {
				cerr << "Failed (passed but was expected to fail)" << endl;
				return 0;
			}
		}
		return 1;
	} catch (UnitTestFailure& ex) {
		if (!expectToFail) {
			cerr << "failed" << endl;
			testExplanation = ex.what();
			return 0;
		} else {
			cerr << "OK (failed but was expected to fail)" << endl;
			return 1;
		}
	} catch (exception& e) {
		if (!expectToFail) {
			cerr << "halted" << endl;
			testExplanation = "Unexpected error in " + currentTest
					+ ": " + e.what();
			return -1;
		} else {
			cerr << "OK (exception but was expected to fail)" << endl;
			return 1;
		}
	} catch (...) {
		if (!expectToFail) {
			cerr << "halted" << endl;
			testExplanation = "Unexpected error in " + currentTest;
			return -1;
		} else {
			cerr << "OK (exception but was expected to fail)" << endl;
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
			cerr << testExplanation << endl;
		} else if (testResult == -1) {
			++numErrors;
			cerr << testExplanation << endl;
		}
	} catch (std::runtime_error& e) {
		++numErrors;
		cerr << "Test " << currentTest << " failed due to "
				<< e.what()
				<< endl;
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
				cerr << testExplanation << endl;
			} else if (testResult == -1) {
				++numErrors;
				cerr << testExplanation << endl;
			}
		} catch (std::runtime_error& e) {
			++numErrors;
			cerr << "Test " << currentTest << " still running after "
					<< timeLimit
					<< " milliseconds - possible infinite loop?"
					<< endl;
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
void UnitTest::runTests (int nTests, char** testNames)
{
	set<string> testsToRun;
	if (nTests > 0) {
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

	} else {
		for (const auto& utest: *tests) {
			testsToRun.insert(utest.first);
		}
	}
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
	UnitTest::runTests(argc-1, argv+1);

	UnitTest::report(cerr);

	return 0;
}

#endif




