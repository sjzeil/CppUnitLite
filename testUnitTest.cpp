/**
 *  Unit test of the unit test framework
 */

#include <exception>
#include <iostream>
#include <string>

#include "unittest.h"


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




