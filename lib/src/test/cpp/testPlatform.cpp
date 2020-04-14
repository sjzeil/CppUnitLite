/**
 *  Unit test of the unit test framework
 */

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>

#include <chrono>
#include <thread>
#include <mutex>


#include "unittest.h"

using namespace std;

UnitTest(testPlatformIdentification) {
#ifdef __MINGW32__
	cout << "# _MINGW32_ is defined" << endl;
#endif
#ifdef __MINGW64__
	cout << "# _MINGW64_ is defined" << endl;
#endif
#ifdef __CYGWIN__
	cout << "# _CYGWIN_ is defined" << endl;
#endif
#ifdef __amd64__
	cout << "# _amd64_ is defined" << endl;
#endif
#ifdef __i386__
	cout << "# _i386_ is defined" << endl;
#endif
    assertTrue(0 == 0);
}


UnitTest(testDebuggerDetection) {
    bool inDebugger = CppUnitLite::UnitTest::debuggerIsRunning();
    assertFalse (inDebugger);
}

UnitTest(testThreadSupport) {
	std::mutex m;
	int timeLimit = 10000;
	std::chrono::duration<int,std::milli> limit (timeLimit);
	std::chrono::duration<int,std::milli> incr (100);
	std::chrono::duration<int,std::milli> elapsed (0);
	int testResult = -1;

	std::thread t([&m, &testResult](){
		{
			int result = 1;
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

	assertThat(testResult, isEqualTo(1));
}
