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
	UnitTest::checkTest (true, "t1", "fileName", 42);
}

UnitTest(testCheckTestFail) {
	UnitTest::expectedToFail();

	UnitTest::checkTest (false, "t1", "fileName", 42);
}


UnitTest(testCheckTestStrPass) {
	UnitTest::checkTest (true, std::string("t1"), "fileName", 42);
}


UnitTest(testCheckTestStrFail) {
	UnitTest::expectedToFail();

	UnitTest::checkTest (false, std::string("t1"), "fileName", 42);
}



void throwException() {
	throw "Catch me if you can";
}

UnitTest(testCheckTestExcept1) {
	UnitTest::expectedToFail();

	throwException();
}


UnitTest(testCatchSegFault) {
	UnitTest::expectedToFail();

	int *p = nullptr;
	assertFalse (*p == 42);
	// Should be caught and recorded as an Error
}



UnitTest(testCatchTestDivideByZero) {
	UnitTest::expectedToFail();

	int k = 12;
	while (k > 0) --k; // may fool busy-body compilers
	int m = 1 / k;
	// Should be caught and recorded as an Error
}


UnitTestTimed(testTimeout1,100) {
#ifndef __MINGW32__
	UnitTest::expectedToFail();
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
	UnitTest::expectedToFail();
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
	UnitTest::logCall("foo");
}

void bar(int a) {
	UnitTest::logCall("bar", a);
}

void baz(int a, bool b) {
	UnitTest::logCall("baz", a, b);
}

void foo(int a, bool b, std::string c) {
	UnitTest::logCall("foo", a, b, c);
}

void bar(int a, bool b, std::string c, double d) {
	UnitTest::logCall("bar", a, b, c, d);
}


UnitTest(testLogging) {
	foo();
	assertEqual (1, distance(UnitTest::begin(), UnitTest::end()));
	assertNotEqual (UnitTest::end(), find(UnitTest::begin(), UnitTest::end(), "foo"));
	UnitTest::clearCallLog();
	bar(21);
	baz(22, true);
	foo(23, false, "hello");
	bar(24, false, "hello", 1.0);

	assertEqual (4, distance(UnitTest::begin(), UnitTest::end()));
	string expected[] = {"bar\t21", "baz\t22\t1", "foo\t23\t0\thello", "bar\t24\t0\thello\t1"};
	assertTrue (equal(UnitTest::begin(), UnitTest::end(), expected));

}

