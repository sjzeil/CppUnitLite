# CppUnitLite - Overview

 This is a lightweight framework similar in spirit to JUnit (for Java),
 Google Test, and Boost Test, but which can be added to a project by the
 simple addition of the two unittest.h and unittest.cpp files.  It's not as
 robust as those other frameworks - some runtime errors will shut the
 test suite down with no final report.
 
# Usage
 
 The framework consists of a two files, `unittest.h` and `unittest.cpp`,
 that can be dropped into
 a C++ project directory, allowing the creation of a unit test suite.
 
 A test suite consists of a collection of unit test functions, which can
 be distributed among multiple .cpp files. (Typically one such file
 would be devoted to testing each class in the project.)
 
 Each unit test function is introduced via `UnitTest` or, optionally,
 `UnitTestTimed` (which alters the default timeout, measured in milliseconds).
 
 Each unit test function can contain code to set up parameters, invoke
 the function(s) being tested, and to evaluate the results of those
 function calls via the use of assertions:
 
       assertTrue (condition);
       assertFalse (condition);
       assertEqual (expression1, expression2);
       assertNotEqual (expression1, expression2);
       assertNull (expression);
       assertNotNull (expression);

For example, a unit test of a simple "counter" class might look like:

     #include "unittest.h"
     #include "myCounter.h"

     UnitTest (testConstructor)
     {
         MyClass x (23);
         assertEqual (23, x.getValue());
         assertFalse (x.isZero());
     }
 
     UnitTestTimed (testIncrement, 100) // Limited to 100ms
     {
         MyClass x (23);
         x.increment();
         assertEqual (24, x.getValue());
         x.increment();
         assertEqual (25, x.getValue());
     }
 
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

      ./unittest testConstructor testIncrement
      ./unittest test
      ./unittest
 
would run both tests.


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
