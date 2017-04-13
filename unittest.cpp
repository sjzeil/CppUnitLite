#include <iostream>
#include <iomanip>
#include <set>
#include <sstream>

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "unittest.h"

using namespace std;

std::map<std::string, UnitTest::BoundedTest> *UnitTest::tests = nullptr;

long UnitTest::numSuccesses = 0L;
long UnitTest::numFailures = 0L;
long UnitTest::numErrors = 0L;
string UnitTest::currentTest;
int UnitTest::timeoutInMilliSec = 500;


UnitTest::UnitTestFailure::UnitTestFailure (
		const char* conditionStr,
		const char* fileName, int lineNumber)
{
	ostringstream out;
	out << "Failed assertion " << conditionStr
			<< " in " << currentTest
			<< " at " << fileName << ", line "
			<< lineNumber;
	explanation = out.str();
}

const char* UnitTest::UnitTestFailure::what() const noexcept {
	return explanation.c_str();
}


void UnitTest::checkTest (bool condition, const char* conditionStr,
		const char* fileName, int lineNumber)
{
	if (!(condition))
	{
		throw UnitTestFailure(conditionStr, fileName, lineNumber);
	}
}

void UnitTest::checkTest (bool condition, const string& conditionStr,
		const char* fileName, int lineNumber)
{
	checkTest(condition, conditionStr.c_str(), fileName, lineNumber);
}



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


int UnitTest::runTestGuarded (std::string testName, TestFunction u,
		std::string& testExplanation)
{
	currentTest = testName;
	cerr << testName << ": " << flush;
	try {
		u();
		cerr << "OK" << endl;
		return 1;
	} catch (UnitTestFailure& ex) {
		cerr << "failed" << endl;
		++numFailures;
		testExplanation = ex.what();
		return 0;
	} catch (exception& e) {
		cerr << "halted" << endl;
		testExplanation = "Unexpected error in " + currentTest
				+ ": " + e.what();
		return -1;
	} catch (...) {
		cerr << "halted" << endl;
		testExplanation = "Unexpected error in " + currentTest;
		return -1;
	}
}

// Run a single unit test function.
void UnitTest::runTest (std::string testName, TestFunction u, int timeLimit)
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
				<< timeoutInMilliSec
				<< " milliseconds - possible infinite loop?"
				<< endl;
	}


}



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
			for (const auto& utest: *tests) {
				if (utest.first.find(testID) != string::npos) {
					testsToRun.insert(utest.first);
				}
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



#ifndef NOMAIN

int main(int argc, char** argv)
{
	UnitTest::runTests(argc-1, argv+1);

	UnitTest::report(cerr);

	return 0;
}

#endif




