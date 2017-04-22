#ifndef UNITTEST_H
#define UNITTEST_H

#include <string>
#include <map>
#include <exception>

/**
 *  This class helps support self-checking unit tests.
 *
 *  This is a lightweight framework similar in spirit to JUnit (for Java),
 *  Google Test, and Boost Test, but which can be added to a project by the
 *  simple addition of the two unittest.h and unittest.cpp files.  It's not as
 *  robust as those other frameworks - some runtime errors will shut the
 *  test suite down with no final report.
 *  
 *  Usage:  A test suite consists of a collection of unit test functions.
 *  Each such function is introduced via UnitTest or, optionally, UnitTestTimed
 *  (which alters the default timeout, measured in milliseconds).
 *
 *  Each unit test function can contain code to set up parameters, invoke
 *  the function(s) being tested, and to evaluate the results of those
 *  function calls via the use of assertions:
 *
 *      assertTrue (condition);
 *      assertFalse (condition);
 *      assertEqual (expression1, expression2);
 *      assertNotEqual (expression1, expression2);
 *      assertNull (expression);
 *      assertNotNull (expression);
 *
 *  For example:
 *
 *    #include "unittest.h"
 *    #include "myCounter.h"
 *
 *    UnitTest (testConstructor)
 *    {
 *        MyClass x (23);
 *        assertEqual (23, x.getValue());
 *        assertFalse (x.isZero());
 *    }
 *
 *    UnitTestTimed (testIncrement, 100) // Limited to 100ms
 *    {
 *        MyClass x (23);
 *        x.increment();
 *        assertEqual (24, x.getValue());
 *        x.increment();
 *        assertEqual (25, x.getValue());
 *    }
 *
 *  The unittest.cpp includes a main() function to drive the tests.  When
 *  run with no command-line parameters, all unit test functions are run.
 *  If command-line parameters are provided, they provide a list of test
 *  function names to indicate which tests to run. Specifically,
 *  any test function whose name contains the command-line parameter
 *  will be run.
 *
 * For example, if the above tests are compiled to form an executable
 * named "unittest", then
 *
 *   ./unittest testIncrement
 *
 * or
 *
 *   ./unittest Incr
 *
 * would run only the second test above, but any of the following
 *
 *   ./unittest testConstructor testIncrement
 *   ./unittest test
 *   ./unittest
 *
 * would run both tests.
 *
 */

class UnitTest {
private:
	static long numSuccesses;
	static long numFailures;
	static long numErrors;
	static std::string currentTest;
	static int timeoutInMilliSec;

public:
	typedef void (*TestFunction)();

	/**
	 * Exception thrown to indicate a failed assertion.
	 */
	class UnitTestFailure: public std::exception {
		std::string explanation;
	public:
		UnitTestFailure (const char* conditionStr,
				const char* fileName, int lineNumber);

		virtual const char* what() const noexcept;
	};

	/**
	 * The main test function - normally called via one of the macros
	 * declared following this class.  Does nothing if the assertion
	 * was passed, but throws an exception if the assertion was failed.
	 *
	 * @param condition the assertion condition, true iff passed.
	 * @param conditionStr a string rendering of the assertion condition.
	 * @param fileName Source code file in which the assertion occurs,
	 * @param lineNumber Source code line number at which the assertion occurs,
	 * @throws UnitTestFailure  if condition is false.
	 */
	static void checkTest (bool condition, const char* conditionStr,
			const char* fileName, int lineNumber);
	/**
	 * The main test function - normally called via one of the macros
	 * declared following this class.  Does nothing if the assertion
	 * was passed, but throws an exception if the assertion was failed.
	 *
	 * @param condition the assertion condition, true iff passed.
	 * @param conditionStr a string rendering of the assertion condition.
	 * @param fileName Source code file in which the assertion occurs,
	 * @param lineNumber Source code line number at which the assertion occurs,
	 * @throws UnitTestFailure  if condition is false.
	 */
	static void checkTest (bool condition, const std::string& conditionStr,
			const char* fileName, int lineNumber);


	// Summary info about tests conducted so far

	/**
	 * How many tests have been run?
	 *
	 * @return number of tests.
	 */
	static long getNumTests()     {return numSuccesses + numFailures;}

	/**
	 * How many tests were terminated by a failed assertion?
	 *
	 * @return number of failed tests.
	 */
	static long getNumFailures()  {return numFailures;}

	/**
	 * How many tests were terminated by an unexpected exception,
	 * run-time error, or time-out?
	 *
	 * @return number of uncompleted tests.
	 */
	static long getNumErrors()  {return numErrors;}

	/**
	 * How many tests terminated successfully?
	 *
	 * @return number of passed tests.
	 */
	static long getNumSuccesses() {return numSuccesses;}


	/**
	 * Set the default time limit for subsequent tests.
	 *
	 * @param timeInMilliSeconds time permitted before a test will
	 *          be forcibly terminated.
	 */
	static void setTimeLimit (int timeInMilliSeconds) {
		timeoutInMilliSec = timeInMilliSeconds;}

	/**
	 * Get the default time limit for subsequent tests.
	 *
	 * @return time permitted before a test will
	 *          be forcibly terminated.
	 */
	static int getTimeLimit() {return timeoutInMilliSec;}

	/**
	 * Run all units tests whose name contains testNames[i],
	 * for all i in 0..nTests-1.
	 *
	 * Special case: If nTests == 0, runs all unit Tests.
	 *
	 * @param nTests number of test name substrings
	 * @param testNames  array of possible substrings of test names
	 */
	static void runTests (int nTests, char** testNames);

	/**
	 * Print a simple summary report. Includes number of tests passed,
	 * failed, and erroneously termnated.
	 *
	 * @param out stream to which to write the report
	 */
	static void report (std::ostream& out);

	/**
	 * Register a new UnitTest, making it eligible for running.
	 *
	 * @param functName name of the test function.
	 * @param timeLimit time limit in milliseconds
	 * @param funct the unit test function
	 */
	static int registerUT (std::string functName, int timeLimit, TestFunction funct);

	/**
	 * Reverses the expectation for the current test.  A test that fails or halts
	 * with an error will be reported and counted as OK.  If that test succeeds,
	 * it will be reported and counted as an error.
	 *
	 * Must be called before any assertions.
	 */
	static void expectedToFail();

	private:
	/**
	 * Internal container for test functions and their associated time limits.
	 */
	struct BoundedTest {
		int timeLimit;
		TestFunction unitTest;

		BoundedTest(): timeLimit(0), unitTest(0) {}
		BoundedTest (int time, TestFunction f): timeLimit(time), unitTest(f) {}
	};
	static std::map<std::string, BoundedTest> *tests;
	static bool expectToFail;

	static void runTest(std::string testName, TestFunction u, int timeLimitInMS);
	static int runTestGuarded(std::string testName, TestFunction u,
			std::string& msg);
};


/**
 *   Macros - actual tests will invoke one of these
 */

#define assertTrue(cond) UnitTest::checkTest (cond, #cond, __FILE__, __LINE__)
#define assertFalse(cond) UnitTest::checkTest (!(cond), std::string("!(") +  #cond + ")", __FILE__, __LINE__)

#define assertEqual( x, y ) UnitTest::checkTest ((x)==(y),\
		"assertEqual("  #x "," #y ")", \
		__FILE__, __LINE__)

#define assertNotEqual( x , y ) assertFalse ((x)==(y))

#define assertNull(x) checkTest ((x)==0)

#define assertNotNull(x) checkTest ((x)!=0)

#define succeed UnitTest::checkTest (true, "succeed", __FILE__, __LINE__)

#define fail UnitTest::checkTest (false, "fail", __FILE__, __LINE__)

/**
 * Test registration
 */

#define UnitTest(functName) void functName(); int functName ## dummy = \
		UnitTest::registerUT(#functName, UnitTest::getTimeLimit(), &functName); void functName()

#define UnitTestTimed(functName, limit) void functName(); int functName ## dummy = \
		UnitTest::registerUT(#functName, limit, &functName); void functName()


#endif
