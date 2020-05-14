#ifndef UNITTEST_H
#define UNITTEST_H

#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

/**
 *  This class helps support self-checking unit tests.
 *
 * This is a lightweight framework similar in spirit to JUnit (for Java),
 *  Google Test, and Boost Test, but which can be added to a project by the
 *  simple addition of the two files, unittest.h and unittest.cpp.  It's not as
 *  robust as those other frameworks - some runtime errors will shut the
 *  test suite down with no final report.
 * 
 * # Usage
 * 
 *  The framework consists of a two files, `unittest.h` and `unittest.cpp`,
 *  that can be dropped into a C++ project directory, allowing the creation of
 *  a unit test suite.
 * 
 *  A test suite consists of a collection of unit test functions, which can
 *  be distributed among multiple .cpp files. (Typically one such file
 *  would be devoted to testing each class in the project.)
 * 
 *  Each unit test function is introduced via `UnitTest` or, optionally,
 *  `UnitTestTimed` (which alters the default timeout, measured in 
 *  milliseconds).
 * 
 *  Each unit test function can contain code to set up parameters, invoke
 *  the function(s) being tested, and to evaluate the results of those
 *  function calls via the use of assertions. Most assertions have the form:
 * 
 *     assertThat (value, matcher);
 * 
 *  although the following "old-fashioned" assertions are also supported.
 * 
 *        assertTrue (condition);
 *        assertFalse (condition);
 *        assertEqual (expression1, expression2);
 *        assertNotEqual (expression1, expression2);
 *        assertNull (expression);
 *        assertNotNull (expression);
 * 
 * The assertThat form, however, allows for a much wider and expressive range of
 * tests:
 * 
 * ## Relational Matchers
 * 
 *     assertThat(x, isEqualTo(y));
 *     assertThat(x, is(y));  // same as isEqualTo
 *     assertThat(x, isApproximately(y, delta));  // floating point only
 *     assertThat(x, isNotEqualTo(y));
 *     assertThat(x, isNot(y));  // same as isNotEqualTo
 * 
 *     assertThat(x, isOneOf(w, y, z));  // Allows 1 or more options
 * 
 *     assertThat(x, isLessThan(y));
 *     assertThat(x, isGreaterThan(y));
 *     assertThat(x, isLessThanOrEqualTo(y));
 *     assertThat(x, isGreaterThanOrEqualTo(y));
 * 
 * ## String Matchers
 * 
 *     assertThat(str, contains("bc"));
 *     assertThat(str, beginsWith(str2));
 *     assertThat(str, endsWith(str2));
 *     assertThat(str, startsWith(str2)); // same as beginsWith
 * 
 * ## Pointer Matchers
 * 
 *     assertThat(p, isNull());
 *     assertThat(q, isNotNull());
 * 
 * ## Container Matchers
 * 
 * Containers that define key_type (sets and maps, including unordered)
 * will be searched using their own fast find member function.  Other
 * containers will be searched using a sequential search over begin()..end().
 * 
 *     assertThat(v, contains(3));
 *     assertThat(v, hasItem(x));  // Same as contains
 *     assertThat(v, hasKey(x));  // Same as contains
 * 
 *     assertThat(L, hasItems(3, 9)); // Allows one or more values
 *     assertThat(L, hasKeys(3, 9));  // Same as hasItems
 * 
 *     assertThat(range(v.begin(), v.end()), hasItem(z));
 *     assertThat(arrayOfLength(array, len), hasItem(z));
 * 
 * 
 *     assertThat(x, isIn(v));
 *     assertThat(x, isInRange(v.begin(), v.end()));
 * 
 *     assertThat(aMap, hasEntry(5, 10)); // maps only
 * 
 * ## Combining Matchers
 * 
 *     assertThat(x, !(matcher));  // Negate a matcher
 * 
 *     assertThat(x, allOf(isLessThan(42), isGreaterThan(10), is(23))); // All must be true
 * 
 *     assertThat(23, anyOf(isLessThan(42), isGreaterThan(10))); // One or more must be true
 * 
 * # Example
 * 
 * ## Writing A Unit Test
 * 
 * A unit test of a simple "counter" class might look like:
 * 
 *      #include "unittest.h"
 *      #include "myCounter.h"
 * 
 *      UnitTest (testConstructor)
 *      {
 *          MyClass x (23);
 *          assertThat (x.getValue(), is(23));
 *          assertThat (x.isZero(), is(true));
 *          assertTrue (x.isZero()); // older style
 *      }
 * 
 *      UnitTestTimed (testIncrement, 100L) // Limited to 100ms
 *      {
 *          MyClass x (23);
 *          x.increment();
 *          assertThat (x.getValue(), is(24));
 *          x.increment();
 *          assertThat (x.getValue(), is(25));
 *      }
 * 
 *      UnitTestTimed (longTest, -1L) // No timer: will never time out
 *      {
 *          MyClass x (23);
 * 	     for (int i = 0; i < 10000; ++i)
 *               x.increment();
 *          assertThat (x.getValue(), is(10023));
 *      }
 * 
 * 
 * ## Running Your Tests
 * 
 * The unittest.cpp includes a main() function to drive the tests.  When
 *  run with no command-line parameters, all unit test functions are run.
 *  If command-line parameters are provided, they provide a list of test
 *  function names to indicate which tests to run.  Specifically,
 *  any test function whose name contains the command-line parameter
 *  will be run.
 * 
 *  For example, if the above tests are compiled to form an executable
 *  named "`unittest`", then
 * 
 *       ./unittest testIncrement
 * 
 * or
 * 
 *       ./unittest Incr
 * 
 * 
 * would run only the second test above, but any of the following
 * 
 *       ./unittest testConstructor testIncrement longTest
 *       ./unittest est
 *       ./unittest
 * 
 * would run all three tests.
 */


/**
 * Time limit, in milliseconds, before a test function is killed
 * on suspicion of the code under test having gone into an infinite loop.
 *
 * Affects all subsequent UniTest(...) declarations, but is ignored by
 * any UnitTestTimed(...) declarations (because they supply their own
 * time limit, overriding the default).
 *
 * Note that a non-positive value for this time limit suppresses the timing
 * check entirely. This may be useful as a way of "stopping the clock"
 * when debugging failed tests.
 */
#define DEFAULT_UNIT_TEST_TIME_LIMIT 500L

/**
 *   Macros - actual tests will invoke one of these
 */

#define assertThat( obj, matcher ) CppUnitLite::UnitTest::checkTest \
	((matcher).eval(obj), \
	 std::string(#obj) + " " + std::string(#matcher), __FILE__, __LINE__)


#define assertTrue(cond) CppUnitLite::UnitTest::checkTest\
	(CppUnitLite::AssertionResult(cond,"",""), #cond, __FILE__, __LINE__)

#define assertTruex(cond) CppUnitLite::UnitTest::checkTest\
	(cond, #cond, __FILE__, __LINE__)

#define assertFalse(cond) CppUnitLite::UnitTest::checkTest\
	(CppUnitLite::AssertionResult(!(cond),"",""), std::string("!(") +  #cond + ")", __FILE__, __LINE__)

#define assertFalsex(cond) CppUnitLite::UnitTest::checkTest\
	(!(cond), std::string("!(") +  #cond + ")", __FILE__, __LINE__)

#define assertEqual( x, y ) assertThat(x, isEqualTo(y))
#define assertEqualx( x, y ) CppUnitLite::UnitTest::checkTest ((x)==(y),\
		"assertEqual("  #x "," #y ")", \
		__FILE__, __LINE__)

#define assertNotEqual( x , y ) assertThat(x, isNotEqualTo(y))

#define assertNotEqualx( x , y ) assertFalse ((x)==(y))

#define assertNull(x) assertTrue ((x)==nullptr)

#define assertNotNull(x) assertTrue ((x)!=nullptr)

#define succeed CppUnitLite::UnitTest::checkTest (\
		CppUnitLite::AssertionResult(true,"",""),\
		"succeed", __FILE__, __LINE__)

#define fail CppUnitLite::UnitTest::checkTest (\
		CppUnitLite::AssertionResult(false,"",""), "fail",\
		__FILE__, __LINE__)

/**
 * Test registration
 */

#define UnitTest(functName) UnitTestTimed(functName, DEFAULT_UNIT_TEST_TIME_LIMIT)

#define UnitTestTimed(functName, limit) void functName(); int functName ## dummy = \
		CppUnitLite::UnitTest::registerUT(#functName, limit, &functName); void functName()





namespace CppUnitLite {




template <typename T>
struct has_begin {
private:
	template <typename U, class = decltype( std::declval<U&>().begin() ) >
	static std::true_type try_begin(U&&);

	static std::false_type try_begin(...);
public:
	using type = decltype( try_begin( std::declval<T>()));
    static constexpr bool value = type();
};

template <typename T>
struct can_be_written {
private:
	template <typename U, class = decltype( std::declval<std::ostringstream&>() << std::declval<U&>()) >
	static std::true_type try_output(U&&);

	static std::false_type try_output(...);
public:
	using type = decltype( try_output( std::declval<T>()));
    static constexpr bool value = type();
};

template <typename T, typename U>
std::string getStringRepr(const std::pair<T,U>&  t);

template <typename T>
std::string getStringRepr(T t);

template<typename T, typename std::enable_if<can_be_written<T>::value, int>::type = 0>
std::string getStringRepr2(T const& t)
{
	std::ostringstream out;
	out << t;
	return out.str();
}



template<typename T, typename std::enable_if<!can_be_written<T>::value && has_begin<T>::value, int>::type = 0>
std::string getStringRepr2(T t)
{
	static const unsigned ContainerDisplayLimit = 10;
	auto n = std::distance(t.begin(), t.end());
	auto pos = t.begin();
	unsigned count = 0;
	std::string result = "[";
	while (pos != t.end() && n > 0)
	{
		result += getStringRepr(*pos);
		if (n > 1)
			result += ", ";
		--n;
		++pos;
		++count;
		if (count >= ContainerDisplayLimit && n > 0)
		{
			result += "...";
			break;
		}
	}
	if (n > 0)
	{
		result += "... (" + getStringRepr(n) + " additional elements) ...";
	}
	result += "]";
	return result;
}

template<typename T, typename std::enable_if<!can_be_written<T>::value && !has_begin<T>::value, int>::type = 0>
std::string getStringRepr2(T t)
{
	return "???";
}

template <typename T, typename U>
std::string getStringRepr(const std::pair<T,U>&  t)
{
	return std::string("<") + getStringRepr(t.first) + ", "
			+ getStringRepr(t.second) + ">";
}

template <typename Tuple, std::size_t size, std::size_t remaining>
struct getTupleRepr
{
	static std::string getContentRepr(Tuple t)  {
		std::string separator = (remaining > 1)? std::string(", ") : std::string();
		return  getStringRepr(std::get<size-remaining>(t))
				+ separator
				+ getTupleRepr<Tuple,size, remaining-1>::getContentRepr(t);
	}
};


template <typename Tuple, std::size_t size>
struct getTupleRepr<Tuple, size, 0>
{
	static std::string getContentRepr(Tuple t)  {
		return  "";
	}
};

template <typename Tuple>
std::string getTupleStringRepr(Tuple t)
{
	return std::string("<")
			+ getTupleRepr<Tuple,
				std::tuple_size<Tuple>::value,
				std::tuple_size<Tuple>::value>::getContentRepr(t)
			+ ">";
}

template <typename... T>
std::string getStringRepr(const std::tuple<T...>&  t)
{
	return getTupleStringRepr(t);
}

template <typename T>
std::string getStringRepr(T t)
{
	return getStringRepr2(t);
}
template <>
std::string getStringRepr(std::string t);
template <>
std::string getStringRepr(const char t[]);
template <>
std::string getStringRepr(char t);
template <>
std::string getStringRepr(bool b);



class AssertionResult {
public:
	bool result; ///> True iff assertion passed
	std::string passExplanation;  ///> Optional explanation for passing;
	std::string failExplanation;  ///> Optional explanation for failure;

	AssertionResult (bool theResult, std::string passExplain,
					 std::string failExplain);

};


/**
 * Main support class for unit test execution.
 */
class UnitTest {
private:
	static long numSuccesses;
	static long numFailures;
	static long numErrors;
	static std::string currentTest;

	static std::vector<std::string> failedTests;
	static std::vector<std::string> callLog;

public:
	/**
	 * Change to false to print diagnostics after the ok/not ok result.
	 */
	static bool diagnosticMessagesBeforeResults;

	typedef void (*TestFunction)();

	/**
	 * Exception thrown to indicate a failed assertion.
	 */
	class UnitTestFailure: public std::exception {
		std::string explanation;
	public:
		UnitTestFailure (const char* conditionStr,
				const char* fileName, int lineNumber);

		UnitTestFailure (const std::string& conditionStr,
				const char* fileName, int lineNumber);

		virtual const char* what() const noexcept;
	};

	/**
	 * The main test function - normally called via one of the macros
	 * declared following this class.  Does nothing if the assertion
	 * was passed, but throws an exception if the assertion was failed.
	 *
	 * @param result the assertion condition, "" if passed, otherwise
	 *                contains an explanation for the failure.
	 * @param conditionStr a string rendering of the assertion condition.
	 * @param fileName Source code file in which the assertion occurs,
	 * @param lineNumber Source code line number at which the assertion occurs,
	 * @throws UnitTestFailure  if condition is false.
	 */
	static void checkTest (AssertionResult result, std::string conditionStr,
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
//	static void checkTest (bool condition, const std::string& conditionStr,
//			const char* fileName, int lineNumber);


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
	 * Run all units tests whose name contains testNames[i],
	 * for all i in 0..nTests-1.
	 *
	 * Special case: If nTests == 0, runs all unit Tests.
	 *
	 * @param nTests number of test name substrings
	 * @param testNames  array of possible substrings of test names
	 * @param programName path to program executable
	 */
	static void runTests (int nTests, char** testNames, char* programName);

	/**
	 * Print a simple summary report. Includes number of tests passed,
	 * failed, and erroneously termnated.
	 *
	 */
	static void report ();

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

	/* ********************************************************
	 * The call log is intended as an aid in writing stubs.
	 * ********************************************************/

	typedef std::vector<std::string>::const_iterator const_iterator;
	typedef std::vector<std::string>::const_iterator iterator;

	/**
	 * Clear the call log.
	 */
	static void clearCallLog();

	/**
	 * Position of oldest logged call.
	 */
	static iterator begin();

	/**
	 * Position just after the most recently logged call.
	 */
	static iterator end();


	/**
	 * Log a call to a zero-parameter function.
	 *
	 * @param functionName name of the function
	 */
	static void logCall (const std::string& functionName);

	/**
	 * Log a call to a function with one parameter.
	 *
	 * Parameter types must support operator<<
	 *
	 * @param functionName name of the function
	 * @param arg1 a parameter to the function call
	 */
	template <typename T1>
	static void logCall (const std::string& functionName, const T1& arg1)
	{
		logCall (functionName + "\t" + getStringRepr(arg1));
	}

	/**
	 * Log a call to a function with two parameters.
	 *
	 * Parameter types must support operator<<
	 *
	 * @param functionName name of the function
	 * @param arg1 a parameter to the function call
	 * @param arg2 a parameter to the function call
	 */
	template <typename T1, typename T2>
	static void logCall (const std::string& functionName,
			const T1& arg1, const T2& arg2)
	{
		logCall (functionName + "\t" + getStringRepr(arg1) + "\t" + getStringRepr(arg2));
	}

	/**
	 * Log a call to a function with three parameters.
	 *
	 * Parameter types must support operator<<
	 *
	 * @param functionName name of the function
	 * @param arg1 a parameter to the function call
	 * @param arg2 a parameter to the function call
	 * @param arg3 a parameter to the function call
	 */
	template <typename T1, typename T2, typename T3>
	static void logCall (const std::string& functionName,
			const T1& arg1, const T2& arg2, const T3& arg3)
	{
		logCall (functionName + "\t" + getStringRepr(arg1) + "\t" + getStringRepr(arg2)
				+ "\t" + getStringRepr(arg3));
	}


	/**
	 * Log a call to a function with four parameters.
	 *
	 * Parameter types must support operator<<
	 *
	 * @param functionName name of the function
	 * @param arg1 a parameter to the function call
	 * @param arg2 a parameter to the function call
	 * @param arg3 a parameter to the function call
	 * @param arg4 a parameter to the function call
	 */
	template <typename T1, typename T2, typename T3, typename T4>
	static void logCall (const std::string& functionName,
			const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4)
	{
		logCall (functionName + "\t" + getStringRepr(arg1) + "\t" + getStringRepr(arg2)
				+ "\t" + getStringRepr(arg3) + "\t" + getStringRepr(arg4));
	}


	// These should be private, but I wanted to unit test them.
	static std::string msgComment (const std::string& commentary);
	static std::string msgFailed (unsigned testNumber, std::string testName, std::string diagnostics, unsigned timeMS);
	static bool debuggerIsRunning();

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

	static void runTest(unsigned testNumber, std::string testName, TestFunction u, long timeLimitInMS);
	static void runTestUntimed(unsigned testNumber, std::string testName, TestFunction u);
	static int runTestGuarded(unsigned testNumber, std::string testName, TestFunction u,
			std::string& msg);

    static std::string extractLocation (const std::string& msg);

	static void msgRunning (unsigned testNumber, std::string testName);
	static void msgPassed (unsigned testNumber, std::string testName, unsigned timeMS);
	static void msgXPassed (unsigned testNumber, std::string testName, unsigned timeMS);
	//static std::string msgFailed (unsigned testNumber, std::string testName, unsigned timeMS);
	//static std::string msgComment (const std::string& commentary);
	static void msgXFailed (unsigned testNumber, std::string testName, std::string diagnostics, unsigned timeMS);
	static void msgError (unsigned testNumber, std::string testName, std::string diagnostics, unsigned timeMS);
	static void msgSummary ();
	static void msg (const std::string& detailMessage);


};

inline void expectedToFail()
{
	UnitTest::expectedToFail();
}

// Compile-time test for associative containers
inline constexpr auto container_has_keytype_impl(...)
    -> std::false_type
{ return std::false_type{}; }

template <typename C, typename = typename C::key_type>
constexpr auto container_has_keytype_impl(C const*) -> std::true_type {
    return std::true_type{};
}

template <typename C>
constexpr auto container_has_keytype(C const& c)
    -> decltype(container_has_keytype_impl(&c))
{
    return container_has_keytype_impl(&c);
}


template <typename Container, typename Element>
long find_in_container_impl (const Container& c, const Element& e, std::false_type)
{
	long ctr = 0L;
	for (auto it = c.begin(); it != c.end(); ++it)
	{
		if (e == *it)
			return ctr;
		++ctr;
	}
	return -1L;
}

template <typename Container, typename Element>
long find_in_container_impl (const Container& c, const Element& e, std::true_type)
{
	auto pos = c.find(e);
	if (c.find(e) != c.end())
		return (long)distance(c.begin(), pos);
	else
		return -1L;
}

template <typename Container, typename Element>
long find_in_container (const Container& c, const Element& e)
{
	return find_in_container_impl (c, e, container_has_keytype(c));
}



//// Relational Matchers

template <typename T>
class EqualToMatcher {
	const T right;
public:
	EqualToMatcher (const T& t)
	: right(t) {}

	/**
	 * Evaluate the condition denoted by this matcher.
	 */
	AssertionResult eval(const T& left) const {
		std::string leftStr = CppUnitLite::getStringRepr(left);
		std::string rightStr = CppUnitLite::getStringRepr(right);
		std::string explain = "Expected: "
				+ rightStr
		        + "\n\tObserved: "
				+ leftStr;

		return AssertionResult(left == right,
					"Both values were: " + getStringRepr(left),
					explain
					);
	}
};

template <typename T, typename U>
class ApproximatelyEqualToMatcher {
	const T right;
	const U delta;
public:
	ApproximatelyEqualToMatcher (const T& t, const U& d): right(t), delta(d) {}
	AssertionResult eval(const T& left) const {
		std::string leftStr = CppUnitLite::getStringRepr(left);
		std::string rightPlusStr = CppUnitLite::getStringRepr(right+delta);
		std::string rightMinusStr = CppUnitLite::getStringRepr(right-delta);
		std::string passExplain = leftStr + " is between " + rightMinusStr
				+ " and " + rightPlusStr;
		if (left < right - delta || left > right + delta)
			return AssertionResult(false,
					passExplain,
					leftStr + " is outside the range "
					+ getStringRepr(right-delta)
					+ " .. "
					+ getStringRepr(right+delta));
		else
			return AssertionResult(true, passExplain, "");
	}
};

template <typename T>
class NotEqualToMatcher {
	const T right;
public:
	NotEqualToMatcher (const T& t): right(t) {}
	AssertionResult eval(const T& left) const {
		std::string leftStr = CppUnitLite::getStringRepr(left);
		std::string rightStr = CppUnitLite::getStringRepr(right);
		std::string explain = "Expected: "
				+ rightStr
		        + "\n\tObserved: "
				+ leftStr;
		return AssertionResult(!(left == right), explain,
					"Both values were: " + getStringRepr(left)
					);
	}
};

template <typename T>
class LessThanMatcher {
	const T right;
public:
	LessThanMatcher (const T& t): right(t) {}
	AssertionResult eval(const T& left) const {
		std::string leftStr = CppUnitLite::getStringRepr(left);
		std::string rightStr = CppUnitLite::getStringRepr(right);
		return AssertionResult(left < right,
					leftStr + " is less than " + rightStr,
					leftStr + " is not less than " + rightStr
					);
	}
};

template <typename T>
class GreaterThanMatcher {
	const T right;
public:
	GreaterThanMatcher (const T& t): right(t) {}
	AssertionResult eval(const T& left) const {
		std::string leftStr = CppUnitLite::getStringRepr(left);
		std::string rightStr = CppUnitLite::getStringRepr(right);
		return AssertionResult(right < left,
					leftStr + " is greater than " + rightStr,
					leftStr + " is not greater than " + rightStr
					);
	}
};

template <typename T>
class LessThanOrEqualToMatcher {
	const T right;
public:
	LessThanOrEqualToMatcher (const T& t): right(t) {}
	AssertionResult eval(const T& left) const {
		std::string leftStr = CppUnitLite::getStringRepr(left);
		std::string rightStr = CppUnitLite::getStringRepr(right);
		return AssertionResult(!(right < left),
					leftStr + " is less than or equal to " + rightStr,
					leftStr + " is greater than " + rightStr
					);
	}
};


template <typename T>
class GreaterThanOrEqualToMatcher {
	const T right;
public:
	GreaterThanOrEqualToMatcher (const T& t): right(t) {}
	AssertionResult eval(const T& left) const {
		std::string leftStr = CppUnitLite::getStringRepr(left);
		std::string rightStr = CppUnitLite::getStringRepr(right);
		return AssertionResult(!(left < right),
					leftStr + " is greater than or equal to " + rightStr,
					leftStr + " is less than " + rightStr
					);
	}
};

//// String Matchers

class StringContainsMatcher {
	const std::string right;
public:
	StringContainsMatcher (const std::string& t);
	AssertionResult eval(const std::string& e) const;
};

class StringEndsWithMatcher {
	const std::string right;
public:
	StringEndsWithMatcher (const std::string& t);
	AssertionResult eval(const std::string& e) const;
};

class StringBeginsWithMatcher {
	const std::string right;
public:
	StringBeginsWithMatcher (const std::string& t);
	AssertionResult eval(const std::string& e) const;
};


// Pointer Matchers

class NullMatcher {
public:
	AssertionResult eval(const void* p) const;
};

class NotNullMatcher {
public:
	AssertionResult eval(const void* p) const;
};


//// Container Matchers




template <typename Element>
class ContainsMatcher {
	Element right;
public:
	ContainsMatcher (Element e) : right(e) {}

	template <typename Container>
	AssertionResult eval(const Container& c) const {
		std::string containerStr = CppUnitLite::getStringRepr(c);
		std::string rightStr = CppUnitLite::getStringRepr(right);
		long pos = find_in_container(c, right);
		return AssertionResult(pos >= 0,
				"Found " + rightStr + " in position " + getStringRepr(pos)
					+ " of " + containerStr,
				"Could not find " + rightStr + " in " + containerStr
				);
	}

};




template <typename Key, typename Data>
class HasEntryMatcher {
	Key key;
	Data data;
public:
	HasEntryMatcher (const Key& k, const Data& d) : key(k), data(d) {}

	template <typename Container>
	AssertionResult eval(const Container& c) const {
		std::string containerStr = CppUnitLite::getStringRepr(c);
		std::string keyStr = CppUnitLite::getStringRepr(key);
		auto pos = c.find(key);
		if (pos != c.end())
		{
			return AssertionResult(data == pos->second,
					"Found " + getStringRepr(*pos)
					+ " in " + containerStr,
					"Could not find <" + keyStr + ", " + getStringRepr(data)
					+ "> in " + containerStr
			);
		}
		else
			return AssertionResult(pos != c.end(),
					"Found " + getStringRepr(*pos)
					+ " in " + containerStr,
					"Could not find " + keyStr + " in " + containerStr
			);
	}

};




template <typename Iterator>
class IteratorRange {
	Iterator start;
	Iterator stop;
public:
	typedef Iterator iterator;
	typedef Iterator const_iterator;

	IteratorRange (Iterator b, Iterator e): start(b), stop(e) {}

	Iterator begin() const { return start; }
	Iterator end() const { return stop; }
};

template <typename... Ts>
class HasItemsMatcher {
	using Element = typename std::common_type<Ts...>::type;
    typename std::vector<Element> right;
public:
    HasItemsMatcher (Ts... ts): right({ts...})
	{ }

    template <typename Container>
    AssertionResult eval (const Container& c) const
    {
    	std::string cStr = getStringRepr(c);
    	std::string foundAll = "Found all of " + getStringRepr(right) + " in " + cStr;
    	for (const Element& e: right)
    	{
    		if (find_in_container(c, e) < 0L) {
    			std::string explain = "Did not find " + getStringRepr(e)
    					+ " in " + cStr;
    			return AssertionResult(false,
    					foundAll,
						explain);
    		}
    	}
    	return AssertionResult(true, foundAll, foundAll);
    }
};


template <typename Iterator1>
class MatchesMatcher {
	const IteratorRange<Iterator1> range1;
public:
	MatchesMatcher (IteratorRange<Iterator1> r1) : range1(r1) {}

	template <typename Iterator2>
	AssertionResult eval(IteratorRange<Iterator2> range2) const {
		auto d1 = std::distance(range1.begin(), range1.end());
		auto d2 = std::distance(range2.begin(), range2.end());
		if (d1 == d2)
		{
			Iterator1 pos1 = range1.begin();
			Iterator2 pos2 = range2.begin();
			while (pos1 != range1.end())
			{
				if (!(*pos1 == *pos2))
				{
					return AssertionResult (false, "",
							"In position "
							+ getStringRepr(std::distance(range1.begin(), pos1))
							+ ", "
							+ getStringRepr(*pos1)
							+ " != " + getStringRepr(*pos2)
							);
				}
				++pos1;
				++pos2;
			}
			return AssertionResult (true, "All corresponding elements were equal.","");
		}
		else
			return AssertionResult(false, "",
					"Ranges are of different length (" + getStringRepr(d1)
					+ " and " + getStringRepr(d2) + ")");
	}
};


template <typename Container>
class IsInMatcher {
	const Container& container;
public:
	IsInMatcher (const Container& c) : container(c) {}

	template <typename Element>
	AssertionResult eval(const Element& e) const {
    	std::string cStr = getStringRepr(container);
    	std::string eStr = getStringRepr(e);
    	long pos = find_in_container(container, e);
		return AssertionResult(pos >= 0L,
				"Found " + eStr + " in postion " + getStringRepr(pos)
					+ " of " + cStr,
				"Could not find " + eStr + " in " + cStr);
	}

};





template <typename Iterator>
class IsInRangeMatcher {
	Iterator start;
	Iterator stop;
public:
	IsInRangeMatcher (Iterator b, Iterator e) : start(b), stop(e) {}

	template <typename Element>
	AssertionResult eval(const Element& e) const {
		std::string eStr = getStringRepr(e);
		auto pos = find(start, stop, e);
		return AssertionResult (pos != stop,
				"Found " + eStr + " in range, "
				   + getStringRepr(distance(start,pos)) + " steps from the start",
				"Could not find " + eStr + " in the range");
	}

};





//// Boolean Matchers

template <typename T>
class NotMatcher {
	T right;
public:
	NotMatcher (const T& t): right(t) {}
	template <typename U>
	AssertionResult eval(const U& u) const {
		AssertionResult r = right.eval(u);
		return AssertionResult(!(r.result), r.failExplanation, r.passExplanation);
	}
};


template <typename... Rest>
class AllOfMatcher {
public:
    AllOfMatcher (Rest... matchers)
	{ }

    template <typename T>
    AssertionResult eval (const T& t) const
    {
    	return AssertionResult(true, "", "");
    }
};
template <typename Matcher, typename... Rest>
class AllOfMatcher<Matcher, Rest...> {
	Matcher matcher;
    AllOfMatcher<Rest...> rest;
public:
    AllOfMatcher (Matcher m, Rest... matchers): matcher(m), rest(matchers...)
	{ }

    template <typename T>
    AssertionResult eval (const T& t) const
    {
    	AssertionResult result1 = matcher.eval(t);
    	if (result1.result)
    	    return rest.eval(t);
    	else
    		return AssertionResult(false, "All of the conditions were true",
    				result1.failExplanation);
    }
};



template <typename... Rest>
class AnyOfMatcher {
public:
    AnyOfMatcher (Rest... matchers)
	{ }

    template <typename T>
    AssertionResult eval (const T& t) const
    {
    	return AssertionResult(false, "", "");
    }
};
template <typename Matcher, typename... Rest>
class AnyOfMatcher<Matcher, Rest...> {
	Matcher matcher;
    AnyOfMatcher<Rest...> rest;
public:
    AnyOfMatcher (Matcher m, Rest... matchers): matcher(m), rest(matchers...)
	{ }

    template <typename T>
    AssertionResult eval (const T& t) const
    {
    	AssertionResult result1 = matcher.eval(t);
    	if (!result1.result)
    			return rest.eval(t);
    	else
    		return AssertionResult(true, result1.passExplanation,
    				"None of the conditions were true");
    }
};



template <typename... T>
class OneOfMatcher {
	using Element = typename std::common_type<T...>::type;
    typename std::vector<Element> right;
public:
    OneOfMatcher (T... t): right({std::forward<T>(t)...})
	{ }

    AssertionResult eval (const Element& left) const
    {
		std::string leftStr = CppUnitLite::getStringRepr(left);
		std::string rightStr = CppUnitLite::getStringRepr(right);
		std::string foundMessage = "Found " + leftStr + " in " + rightStr;
		std::string notFoundMessage = "Could not find " + leftStr + " in " + rightStr;
    	for (const Element& e: right)
    	{
    		if (left == e) return AssertionResult(true, foundMessage, notFoundMessage);
    	}
    	return AssertionResult(false, foundMessage, notFoundMessage);
    }
};


}




////  Matchers


/// Relational Matchers

inline CppUnitLite::EqualToMatcher<std::string>
isEqualTo(const char* t)
{
	return CppUnitLite::EqualToMatcher<std::string>(std::string(t));
}

template <typename T>
CppUnitLite::EqualToMatcher<T>
isEqualTo(const T& t)
{
	return CppUnitLite::EqualToMatcher<T>(t);
}

inline CppUnitLite::EqualToMatcher<std::string>
is(const char* t)
{
	return CppUnitLite::EqualToMatcher<std::string>(std::string(t));
}

template <typename T>
CppUnitLite::EqualToMatcher<T>
is(const T& t)
{
	return CppUnitLite::EqualToMatcher<T>(t);
}

template <typename T, typename U>
CppUnitLite::ApproximatelyEqualToMatcher<T,U>
isApproximately(const T& t, const U& delta)
{
	return CppUnitLite::ApproximatelyEqualToMatcher<T,U>(t, delta);
}

inline CppUnitLite::NotEqualToMatcher<std::string>
isNotEqualTo(const char* t)
{
	return CppUnitLite::NotEqualToMatcher<std::string>(std::string(t));
}

template <typename T>
CppUnitLite::NotEqualToMatcher<T>
isNotEqualTo(const T& t)
{
	return CppUnitLite::NotEqualToMatcher<T>(t);
}

inline CppUnitLite::NotEqualToMatcher<std::string>
isNot(const char* t)
{
	return CppUnitLite::NotEqualToMatcher<std::string>(std::string(t));
}

template <typename T>
CppUnitLite::NotEqualToMatcher<T>
isNot(const T& t)
{
	return CppUnitLite::NotEqualToMatcher<T>(t);
}

inline CppUnitLite::LessThanMatcher<std::string> \
isLessThan(const char* t)
{
	return CppUnitLite::LessThanMatcher<std::string>(std::string(t));
}

template <typename T>
CppUnitLite::LessThanMatcher<T> isLessThan(const T& t)
{
	return CppUnitLite::LessThanMatcher<T>(t);
}

inline CppUnitLite::GreaterThanMatcher<std::string>
isGreaterThan(const char* t)
{
	return CppUnitLite::GreaterThanMatcher<std::string>(std::string(t));
}

template <typename T>
CppUnitLite::GreaterThanMatcher<T>
isGreaterThan(const T& t)
{
	return CppUnitLite::GreaterThanMatcher<T>(t);
}

inline CppUnitLite::LessThanOrEqualToMatcher<std::string>
isLessThanOrEqualTo(const char* t)
{
	return CppUnitLite::LessThanOrEqualToMatcher<std::string>(std::string(t));
}
template <typename T>
CppUnitLite::LessThanOrEqualToMatcher<T>
isLessThanOrEqualTo(const T& t)
{
	return CppUnitLite::LessThanOrEqualToMatcher<T>(t);
}

inline CppUnitLite::GreaterThanOrEqualToMatcher<std::string>
isGreaterThanOrEqualTo(const char* t)
{
	return CppUnitLite::GreaterThanOrEqualToMatcher<std::string>(std::string(t));
}

template <typename T>
CppUnitLite::GreaterThanOrEqualToMatcher<T>
isGreaterThanOrEqualTo(const T& t)
{
	return CppUnitLite::GreaterThanOrEqualToMatcher<T>(t);
}


/// String matchers

CppUnitLite::StringContainsMatcher contains(const char* t);
CppUnitLite::StringContainsMatcher contains(const std::string& t);

CppUnitLite::StringEndsWithMatcher endsWith(const char* t);
CppUnitLite::StringEndsWithMatcher endsWith(const std::string& t);

CppUnitLite::StringBeginsWithMatcher beginsWith(const char* t);
CppUnitLite::StringBeginsWithMatcher beginsWith(const std::string& t);
CppUnitLite::StringBeginsWithMatcher startsWith(const char* t);
CppUnitLite::StringBeginsWithMatcher startsWith(const std::string& t);


/// Pointer matchers

CppUnitLite::NullMatcher isNull();
CppUnitLite::NotNullMatcher isNotNull();


/// Container matchers


template <typename T>
CppUnitLite::ContainsMatcher<T> hasItem(const T& e)
{
	return CppUnitLite::ContainsMatcher<T>(e);
}

template <typename T>
CppUnitLite::ContainsMatcher<T> contains(const T& e)
{
	return CppUnitLite::ContainsMatcher<T>(e);
}

template <typename T>
CppUnitLite::ContainsMatcher<T> hasKey(const T& e)
{
	return CppUnitLite::ContainsMatcher<T>(e);
}

template <typename Key, typename Data>
CppUnitLite::HasEntryMatcher<Key, Data> hasEntry(const Key& k, const Data& d)
{
	return CppUnitLite::HasEntryMatcher<Key, Data>(k, d);
}

template <typename Element>
CppUnitLite::IteratorRange<const Element*>
arrayOfLength (const Element* start, int n)
{
	return CppUnitLite::IteratorRange<const Element*>(start, start+n);
}
template <typename Iterator>
CppUnitLite::IteratorRange<Iterator> range (Iterator start, Iterator stop)
{
	return CppUnitLite::IteratorRange<Iterator>(start, stop);
}

template <typename... Ts>
CppUnitLite::HasItemsMatcher<Ts...> hasItems (Ts... t)
{
	return CppUnitLite::HasItemsMatcher<Ts...>(t...);
}

template <typename T>
CppUnitLite::MatchesMatcher<T> matches (CppUnitLite::IteratorRange<T> range)
{
	return CppUnitLite::MatchesMatcher<T>(range);
}


/// Associative container (set & map) matchers

template <typename... Ts>
CppUnitLite::HasItemsMatcher<Ts...> hasKeys (Ts... t)
{
	return CppUnitLite::HasItemsMatcher<Ts...>(t...);
}

template <typename Container>
CppUnitLite::IsInMatcher<Container> isIn(const Container& c)
{
	return CppUnitLite::IsInMatcher<Container>(c);
}

template <typename Iterator>
CppUnitLite::IsInRangeMatcher<Iterator> isInRange(Iterator b, Iterator e)
{
	return CppUnitLite::IsInRangeMatcher<Iterator>(b, e);
}


/// Combining matchers

template <typename T>
CppUnitLite::NotMatcher<T>
operator!(const T& t)
{
	return CppUnitLite::NotMatcher<T>(t);
}

template <typename... Ts>
CppUnitLite::AllOfMatcher<Ts...>
allOf(Ts... ts)
{
	return CppUnitLite::AllOfMatcher<Ts...>(ts...);
}

template <typename... Ts>
CppUnitLite::AnyOfMatcher<Ts...>
anyOf(Ts... ts)
{
	return CppUnitLite::AnyOfMatcher<Ts...>(ts...);
}

template <typename... T>
CppUnitLite::OneOfMatcher<T...>
isOneOf (T... t)
{
	return CppUnitLite::OneOfMatcher<T...>(t...);
}





#endif
