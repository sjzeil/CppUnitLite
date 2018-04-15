# CppUnitLite - Overview

 This is a lightweight framework similar in spirit to JUnit (for Java),
 Google Test, and Boost Test, but which can be added to a project by the
 simple addition of the two files, unittest.h and unittest.cpp.  It's not as
 robust as those other frameworks - some runtime errors will shut the
 test suite down with no final report.
 
# Usage
 
 The framework consists of a two files, `unittest.h` and `unittest.cpp`,
 that can be dropped into a C++ project directory, allowing the creation of
 a unit test suite.
 
 A test suite consists of a collection of unit test functions, which can
 be distributed among multiple .cpp files. (Typically one such file
 would be devoted to testing each class in the project.)
 
 Each unit test function is introduced via `UnitTest` or, optionally,
 `UnitTestTimed` (which alters the default timeout, measured in milliseconds).
 
 Each unit test function can contain code to set up parameters, invoke
 the function(s) being tested, and to evaluate the results of those
 function calls via the use of assertions. Most assertions have the form:
 
    assertThat (value, matcher);
 
 although the following "old-fashioned" assertions are also supported.
 
       assertTrue (condition);
       assertFalse (condition);
       assertEqual (expression1, expression2);
       assertNotEqual (expression1, expression2);
       assertNull (expression);
       assertNotNull (expression);

The assertThat form, however, allows for a much wider and expressive range of
tests:

## Relational Matchers

    assertThat(x, isEqualTo(y));
    assertThat(x, is(y));  // same as isEqualTo
    assertThat(x, isApproximately(y, delta));  // floating point only
    assertThat(x, isNotEqualTo(y));
    assertThat(x, isNot(y));  // same as isNotEqualTo

    assertThat(x, isOneOf(w, y, z));  // Allows 1 or more options
    
    assertThat(x, isLessThan(y));
    assertThat(x, isGreaterThan(y));
    assertThat(x, isLessThanOrEqualTo(y));
    assertThat(x, isGreaterThanOrEqualTo(y));

## String Matchers

    assertThat(str, contains("bc"));
    assertThat(str, beginsWith(str2));
    assertThat(str, endsWith(str2));
    assertThat(str, startsWith(str2)); // same as beginsWith

## Pointer Matchers

    assertThat(p, isNull());
    assertThat(q, isNotNull());

## Container Matchers

Containers that define key_type (sets and maps, including unordered)
will be searched using their own fast find member function.  Other
containers will be searched using a sequential search over begin()..end().

    assertThat(v, contains(3));
    assertThat(v, hasItem(x));  // Same as contains
    assertThat(v, hasKey(x));  // Same as contains

    assertThat(L, hasItems(3, 9)); // Allows one or more values
    assertThat(L, hasKeys(3, 9));  // Same as hasItems

    assertThat(range(v.begin(), v.end()), hasItem(z));
    assertThat(arrayOfLength(array, len), hasItem(z));

    assertThat(x, isIn(v));
    assertThat(x, isInRange(v.begin(), v.end()));

    assertThat(aMap, hasEntry(5, 10)); // maps only


## Combining Matchers

    assertThat(x, !(matcher));  // Negate a matcher

    assertThat(x, allOf(isLessThan(42), isGreaterThan(10), is(23))); // All must be true

    assertThat(23, anyOf(isLessThan(42), isGreaterThan(10))); // One or more must be true

# Example

## Writing A Unit Test

A unit test of a simple "counter" class might look like:

     #include "unittest.h"
     #include "myCounter.h"

     UnitTest (testConstructor)
     {
         MyClass x (23);
         assertThat (x.getValue(), is(23));
         assertThat (x.isZero(), is(true));
         assertTrue (x.isZero()); // older style
     }
 
     UnitTestTimed (testIncrement, 100L) // Limited to 100ms
     {
         MyClass x (23);
         x.increment();
         assertThat (x.getValue(), is(24));
         x.increment();
         assertThat (x.getValue(), is(25));
     }
 
     UnitTestTimed (longTest, -1L) // No timer: will never time out
     {
         MyClass x (23);
	     for (int i = 0; i < 10000; ++i)
              x.increment();
         assertThat (x.getValue(), is(10023));
     }

## Running Your Tests

The unittest.cpp includes a main() function to drive the tests.  When
 run with no command-line parameters, all unit test functions are run.
 If command-line parameters are provided, they provide a list of test
 function names to indicate which tests to run.  Specifically,
 any test function whose name contains the command-line parameter
 will be run.
 
 For example, if the above tests are compiled to form an executable
 named "`unittest`", then

      ./unittest testIncrement
      
or

      ./unittest Incr


would run only the second test above, but any of the following

      ./unittest testConstructor testIncrement longTest
      ./unittest est
      ./unittest
 
would run all three tests.


# Compiling Unit Tests

If your project has its own `main()` function, be aware that this
adds another one to your project. The default project managers in
many C++ IDEs will not know how to handle projects with more than
one `main()` function and will report errors when trying to link
the code.   Workarounds:

* If your IDE has an option for using `make` in place of its
  default project manager, you can construct a makefile to
  build both executables (the "real" project and the unit test)
  separately.

* You may be able to put the `unittest.*` files in a subdirectory
  of the main project and create a new IDE project in that subdirectory,
  sharing the non-`main()` project source files from the parent directory
  with the new unit test project.

This framework uses threads to implement a time-out function for tests
of buggy code that may be caught in an infinite loop. On Unix-based
`g++` systems, this requires adding -pthread` as a command option in the
final linkage step, e.g.,

    g++ -o unittest -g unittest.o adt1.o adt2.o -pthread
    
On Windows systems, the timing functions may not work. In particular, they are
ignored when a MinGW compiler is used.

# Debugging Unit Tests

One of the major benefits of the *Unit style of testing is that it provides
easy launch points for debugging.  Some tips to consider:

* Turn off the timing function when debugging. Obviously, if your unit
   tests fail automatically when you spend morethan 0.5 seconds (the default) 
   or any other short time, this will interfere with using a debugger to set
   break points, step through code, etc.   You can turn off the timing by
   placing
   
        #define DEFAULT_UNIT_TEST_TIME_LIMIT -1L 

    in front of your `UnitTest`s.  The negative value disables the default
    time-out behavior.  
   
* If you are setting breakpoints
  inside your code, you may find it distracting that these
  breakpoints are hit multiple times by unit tests that you are passing before
  execution even begins on the one(s) that you have failed.
  
    Use the command line arguments (see **Running Your Tests**, above) to
    limit your debugging runs to the test(s) that you are actually failing
    so as to focus your debugging efforts.
  
  
  
