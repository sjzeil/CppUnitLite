/**
 *  Unit test of the unit test framework
 */

#include <exception>
#include <string>

#include "unittest.h"


int nErr0 = 0;


UnitTest(testCheckTest) {
  int nTests0 = UnitTest::getNumTests();
  int nFail0 = UnitTest::getNumFailures();
  nErr0 = UnitTest::getNumErrors();
  int nSucc0 = UnitTest::getNumSuccesses();

  UnitTest::checkTest (true, "t1", "fileName", 42);

  assertEqual (nTests0+1, UnitTest::getNumTests());
  assertEqual (nFail0, UnitTest::getNumFailures());
  assertEqual (nErr0, UnitTest::getNumErrors());
  assertEqual (nSucc0+1, UnitTest::getNumSuccesses());

  UnitTest::checkTest (false, "t1", "fileName", 42);

  assertEqual (nTests0+2, UnitTest::getNumTests());
  assertEqual (nFail0+1, UnitTest::getNumFailures());
  assertEqual (nErr0, UnitTest::getNumErrors());
  assertEqual (nSucc0+1, UnitTest::getNumSuccesses());

}

UnitTest(testCheckTestStr) {
  int nTests0 = UnitTest::getNumTests();
  int nFail0 = UnitTest::getNumFailures();
  nErr0 = UnitTest::getNumErrors();
  int nSucc0 = UnitTest::getNumSuccesses();

  UnitTest::checkTest (true, std::string("t1"), "fileName", 42);

  assertEqual (nTests0+1, UnitTest::getNumTests());
  assertEqual (nFail0, UnitTest::getNumFailures());
  assertEqual (nErr0, UnitTest::getNumErrors());
  assertEqual (nSucc0+1, UnitTest::getNumSuccesses());

  UnitTest::checkTest (false, std::string("t1"), "fileName", 42);

  assertEqual (nTests0+2, UnitTest::getNumTests());
  assertEqual (nFail0+1, UnitTest::getNumFailures());
  assertEqual (nErr0, UnitTest::getNumErrors());
  assertEqual (nSucc0+1, UnitTest::getNumSuccesses());

}

void throwException() {
  throw "Catch me if you can";
}

UnitTest(testCheckTestExcept1) {
  nErr0 = UnitTest::getNumErrors();
  throwException();
  // Should be recorded as an Error
}

UnitTest(testCheckTestExcept2) {
  // Will run after testCheckTestExcept1 (relies on undocumented
  //   and unguaranteed-in-the-future property that tests are run
  // in alphabetical order).
  assertEqual (nErr0+1, UnitTest::getNumErrors());
}

UnitTest(testCheckTestExcept3) {
  nErr0 = UnitTest::getNumErrors();
  int *p = nullptr;
  assertFalse (*p == 42);
  // Should be caught and recorded as an Error
}

UnitTest(testCheckTestExcept4) {
  // Will run after testCheckTestExcept3 (relies on undocumented
  //   and unguaranteed-in-the-future property that tests are run
  // in alphabetical order).
  assertEqual (nErr0+1, UnitTest::getNumErrors());
}


UnitTest(testCheckTestExcept5) {
  nErr0 = UnitTest::getNumErrors();
  int k = 12;
  while (k > 0) --k; // may fool busy-body compilers
  int m = 1 / k;
  // Should be caught and recorded as an Error
}

UnitTest(testCheckTestExcept6) {
  // Will run after testCheckTestExcept3 (relies on undocumented
  //   and unguaranteed-in-the-future property that tests are run
  // in alphabetical order).
  assertEqual (nErr0+1, UnitTest::getNumErrors());
}


UnitTestTimed(TestTimeout1,100) {
  nErr0 = UnitTest::getNumErrors();
  long k = 0;
  for (int i = 0; i < 100000; ++i) {
    ++k;
    for (int j = 0; j < 100000; ++j) {
      ++k;
    }
  }
}

UnitTestTimed(TestTimeout2,100) {
  assertEqual (nErr0+1, UnitTest::getNumErrors());
}



///////////////  sample sig handler  ///////////


#include <signal.h>

#include <stdio.h>
#include <unistd.h>

void ouch(int sig)
{
   printf(“OUCH! - I got signal %d\n”, sig);
}
int sigs()
{
   struct sigaction act;
   act.sa_handler = ouch;
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   sigaction(SIGINT, &act, 0);
   while(1) {
      printf(“Hello World!\n”);
      sleep(1);
   }
}
