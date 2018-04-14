/**
 *  Unit test of the unit test framework
 */

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>

#include "unittest.h"

using namespace std;



UnitTest(testCheckTestPass) {
	CppUnitLite::UnitTest::checkTest (true, "t1", "fileName", 42);
}

UnitTest(testCheckTestFail) {
	CppUnitLite::UnitTest::expectedToFail();

	CppUnitLite::UnitTest::checkTest (false, "t1", "fileName", 42);
}


UnitTest(testCheckTestStrPass) {
	CppUnitLite::UnitTest::checkTest (true, std::string("t1"), "fileName", 42);
}


UnitTest(testCheckTestStrFail) {
	CppUnitLite::UnitTest::expectedToFail();

	CppUnitLite::UnitTest::checkTest (false, std::string("t1"), "fileName", 42);
}



void throwException() {
	throw "Catch me if you can";
}

UnitTest(testCheckTestExcept1) {
	CppUnitLite::UnitTest::expectedToFail();

	throwException();
}


UnitTest(testCatchSegFault) {
	CppUnitLite::UnitTest::expectedToFail();

	int *p = nullptr;
	assertFalse (*p == 42);
	// Should be caught and recorded as an Error
}



UnitTest(testCatchTestDivideByZero) {
	CppUnitLite::UnitTest::expectedToFail();

	int k = 12;
	while (k > 0) --k; // may fool busy-body compilers
	int m = 1 / k;
	// Should be caught and recorded as an Error
}


UnitTestTimed(testTimeout1,100) {
#ifndef __MINGW32__
	CppUnitLite::UnitTest::expectedToFail();
	long k = 0;
	for (int i = 0; i < 100000; ++i) {
		++k;
		for (int j = 0; j < 100000; ++j) {
			++k;
		}
	}
#else
	std::cerr << "Timeouts cannot be detected in compilers with no thread support"
			<< std::endl;
	UnitTest::fail;
#endif
}

// Example of overriding default limit
#define DEFAULT_UNIT_TEST_TIME_LIMIT 1000L

UnitTest(testTimeoutDefaultLimit) {
#ifndef __MINGW32__
	CppUnitLite::expectedToFail();
	long k = 0;
	for (int i = 0; i < 100000; ++i) {
		++k;
		for (int j = 0; j < 100000; ++j) {
			++k;
		}
	}
#else
	std::cerr << "Timeouts cannot be detected in compilers with no thread support"
			<< std::endl;
	UnitTest::fail;
#endif
}
void foo() {
	CppUnitLite::UnitTest::logCall("foo");
}

void bar(int a) {
	CppUnitLite::UnitTest::logCall("bar", a);
}

void baz(int a, bool b) {
	CppUnitLite::UnitTest::logCall("baz", a, b);
}

void foo(int a, bool b, std::string c) {
	CppUnitLite::UnitTest::logCall("foo", a, b, c);
}

void bar(int a, bool b, std::string c, double d) {
	CppUnitLite::UnitTest::logCall("bar", a, b, c, d);
}


UnitTest(testLogging) {
	foo();
	assertEqual (1, distance(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end()));
	assertNotEqual (CppUnitLite::UnitTest::end(), find(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end(), "foo"));
	CppUnitLite::UnitTest::clearCallLog();
	bar(21);
	baz(22, true);
	foo(23, false, "hello");
	bar(24, false, "hello", 1.0);

	assertEqual (4, distance(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end()));
	string expected[] = {"bar\t21", "baz\t22\t1", "foo\t23\t0\thello", "bar\t24\t0\thello\t1"};
	assertTrue (equal(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end(), expected));

}

