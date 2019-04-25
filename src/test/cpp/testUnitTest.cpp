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
	CppUnitLite::UnitTest::checkTest (CppUnitLite::AssertionResult(true, "", ""), "t1", "fileName", 42);
}

UnitTest(testCheckTestFail) {
	CppUnitLite::UnitTest::expectedToFail();

	CppUnitLite::UnitTest::checkTest (CppUnitLite::AssertionResult(false, "", ""), "t1", "fileName", 42);
}


UnitTest(testCheckTestStrPass) {
	CppUnitLite::UnitTest::checkTest (CppUnitLite::AssertionResult(true, "", ""), std::string("t1"), "fileName", 42);
}


UnitTest(testCheckTestStrFail) {
	CppUnitLite::UnitTest::expectedToFail();

	CppUnitLite::UnitTest::checkTest (CppUnitLite::AssertionResult(false, "", ""), std::string("t1"), "fileName", 42);
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
	assertTrue (m < m + 1);
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
	fail;
#endif
}

// Example of overriding default limit
#undef DEFAULT_UNIT_TEST_TIME_LIMIT
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
	fail;
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
	CppUnitLite::UnitTest::clearCallLog();
	foo();
	assertEqual (1, distance(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end()));
	assertNotEqual (CppUnitLite::UnitTest::end(), find(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end(), "foo"));
	CppUnitLite::UnitTest::clearCallLog();
	bar(21);
	baz(22, true);
	foo(23, false, "hello");
	bar(24, false, "hello", 1.0);

	assertEqual (4, distance(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end()));
	std::array<string, 4> expected {"bar\t21", "baz\t22\ttrue", "foo\t23\tfalse\t\"hello\"", "bar\t24\tfalse\t\"hello\"\t1"};
	assertThat (range(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end()),
			matches(range(expected.begin(), expected.end())));

}

class FooBar { int i;};

void foobar(const FooBar fb) {
	CppUnitLite::UnitTest::logCall("foobar", fb);
}

UnitTest(testLoggingUnprintable) {
	CppUnitLite::UnitTest::clearCallLog();
	foobar(FooBar());

	assertEqual (1, distance(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end()));
	std::array<string,1> expected {"foobar\t???"};
	assertThat (range(CppUnitLite::UnitTest::begin(), CppUnitLite::UnitTest::end()),
			matches(range(expected.begin(), expected.end())));

}


