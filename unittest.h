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
	(cond, #cond, __FILE__, __LINE__)

#define assertFalse(cond) CppUnitLite::UnitTest::checkTest\
	(!(cond), std::string("!(") +  #cond + ")", __FILE__, __LINE__)

#define assertEqual( x, y ) CppUnitLite::UnitTest::checkTest ((x)==(y),\
		"assertEqual("  #x "," #y ")", \
		__FILE__, __LINE__)

#define assertNotEqual( x , y ) assertFalse ((x)==(y))

#define assertNull(x) assertTrue ((x)==nullptr)

#define assertNotNull(x) assertTrue ((x)!=nullptr)

#define succeed CppUnitLite::UnitTest::checkTest (true, "succeed", __FILE__, __LINE__)

#define fail CppUnitLite::UnitTest::checkTest (false, "fail", __FILE__, __LINE__)

/**
 * Test registration
 */

#define UnitTest(functName) UnitTestTimed(functName, DEFAULT_UNIT_TEST_TIME_LIMIT)

#define UnitTestTimed(functName, limit) void functName(); int functName ## dummy = \
		CppUnitLite::UnitTest::registerUT(#functName, limit, &functName); void functName()





namespace CppUnitLite {
/**
 * Main support class for unit test execution.
 */
class UnitTest {
private:
	static long numSuccesses;
	static long numFailures;
	static long numErrors;
	static std::string currentTest;

	static std::vector<std::string> callLog;

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
		using namespace std;
		ostringstream out;
		out << functionName;
		out << "\t" << arg1;
		logCall (out.str());
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
		using namespace std;
		ostringstream out;
		out << functionName;
		out << "\t" << arg1;
		out << "\t" << arg2;
		logCall (out.str());
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
		using namespace std;
		ostringstream out;
		out << functionName;
		out << "\t" << arg1;
		out << "\t" << arg2;
		out << "\t" << arg3;
		logCall (out.str());
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
		using namespace std;
		ostringstream out;
		out << functionName;
		out << "\t" << arg1;
		out << "\t" << arg2;
		out << "\t" << arg3;
		out << "\t" << arg4;
		logCall (out.str());
	}


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

	static void runTest(std::string testName, TestFunction u, long timeLimitInMS);
	static void runTestUntimed(std::string testName, TestFunction u);
	static int runTestGuarded(std::string testName, TestFunction u,
			std::string& msg);
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
bool find_in_container_impl (const Container& c, const Element& e, std::false_type)
{
	for (auto it = c.begin(); it != c.end(); ++it)
		if (e == *it)
			return true;
	return false;
}

template <typename Container, typename Element>
bool find_in_container_impl (const Container& c, const Element& e, std::true_type)
{
	return c.find(e) != c.end();
}

template <typename Container, typename Element>
bool find_in_container (const Container& c, const Element& e)
{
	return find_in_container_impl (c, e, container_has_keytype(c));
}



//// Relational Matchers

template <typename T>
class EqualToMatcher {
	const T hold;
public:
	EqualToMatcher (const T& t): hold(t) {}
	bool eval(const T& t) const {
		return t == hold;
	}
};

template <typename T>
class NotEqualToMatcher {
	const T hold;
public:
	NotEqualToMatcher (const T& t): hold(t) {}
	bool eval(const T& t) const {
		return !(t == hold);
	}
};

template <typename T>
class LessThanMatcher {
	const T hold;
public:
	LessThanMatcher (const T& t): hold(t) {}
	bool eval(const T& t) const {
		return t < hold;
	}
};

template <typename T>
class GreaterThanMatcher {
	const T hold;
public:
	GreaterThanMatcher (const T& t): hold(t) {}
	bool eval(const T& t) const {
		return hold < t;
	}
};

template <typename T>
class LessThanOrEqualToMatcher {
	const T hold;
public:
	LessThanOrEqualToMatcher (const T& t): hold(t) {}
	bool eval(const T& t) const {
		return !(hold < t);
	}
};


template <typename T>
class GreaterThanOrEqualToMatcher {
	const T hold;
public:
	GreaterThanOrEqualToMatcher (const T& t): hold(t) {}
	bool eval(const T& t) const {
		return !(t < hold);
	}
};

//// String Matchers

class StringContainsMatcher {
	const std::string hold;
public:
	StringContainsMatcher (const std::string& t);
	bool eval(const std::string& e) const;
};

class StringEndsWithMatcher {
	const std::string hold;
public:
	StringEndsWithMatcher (const std::string& t);
	bool eval(const std::string& e) const;
};

class StringBeginsWithMatcher {
	const std::string hold;
public:
	StringBeginsWithMatcher (const std::string& t);
	bool eval(const std::string& e) const;
};


// Pointer Matchers

class NullMatcher {
public:
	bool eval(void* p) const;
};

class NotNullMatcher {
public:
	bool eval(void* p) const;
};


//// Container Matchers




template <typename Element>
class ContainsMatcher {
	Element hold;
public:
	ContainsMatcher (Element e) : hold(e) {}

	template <typename Container>
	bool eval(const Container& c) const {
		return find_in_container(c, hold);
	}

};




template <typename Key, typename Data>
class HasEntryMatcher {
	Key key;
	Data data;
public:
	HasEntryMatcher (const Key& k, const Data& d) : key(k), data(d) {}

	template <typename Container>
	bool eval(const Container& c) const {
		auto pos = c.find(key);
		if (pos != c.end())
		{
			return (pos->second == data);
		}
		else
		{
			return false;
		}
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
    typename std::vector<Element> hold;
public:
    HasItemsMatcher (Ts... ts): hold({ts...})
	{ }

    template <typename Container>
    bool eval (const Container& c) const
    {
    	for (const Element& e: hold)
    	{
    		if (!find_in_container(c, e)) return false;
    	}
    	return true;
    }
};




template <typename Container>
class IsInMatcher {
	const Container& container;
public:
	IsInMatcher (const Container& c) : container(c) {}

	template <typename Element>
	bool eval(const Element& e) const {
		return find_in_container(container, e);
	}

};





template <typename Iterator>
class IsInRangeMatcher {
	Iterator start;
	Iterator stop;
public:
	IsInRangeMatcher (Iterator b, Iterator e) : start(b), stop(e) {}

	template <typename Element>
	bool eval(const Element& e) const {
		return find(start, stop, e) != stop;
	}

};





//// Boolean Matchers

template <typename T>
class NotMatcher {
	T hold;
public:
	NotMatcher (const T& t): hold(t) {}
	template <typename U>
	bool eval(const U& u) const {
		return !(hold.eval(u));
	}
};


template <typename... Rest>
class AllOfMatcher {
public:
    AllOfMatcher (Rest... matchers)
	{ }

    template <typename T>
    bool eval (const T& t) const
    {
    	return true;
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
    bool eval (const T& t) const
    {
    	return matcher.eval(t) && rest.eval(t);
    }
};



template <typename... Rest>
class AnyOfMatcher {
public:
    AnyOfMatcher (Rest... matchers)
	{ }

    template <typename T>
    bool eval (const T& t) const
    {
    	return false;
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
    bool eval (const T& t) const
    {
    	return matcher.eval(t) || rest.eval(t);
    }
};




template <typename... T>
class OneOfMatcher {
	using Element = typename std::common_type<T...>::type;
    typename std::vector<Element> hold;
public:
    OneOfMatcher (T... t): hold({std::forward<T>(t)...})
	{ }

    bool eval (const Element& t) const
    {
    	for (const Element& e: hold)
    	{
    		if (t == e) return true;
    	}
    	return false;
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
