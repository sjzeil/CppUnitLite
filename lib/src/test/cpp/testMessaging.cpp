/**
 *  Unit test of the unit test framework
 */

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>

#include "unittest.h"

using namespace std;


UnitTest(testCommentaryUnchanged) {
    string msg1 = "# already in commentary \n# form.";
    string result1 = CppUnitLite::UnitTest::msgComment(msg1);
	assertThat (result1, isEqualTo(msg1));
}

UnitTest(testCommentaryOneLine) {
    string msg1 = "something happened";
    string expected1 = "# something happened";
    string result1 = CppUnitLite::UnitTest::msgComment(msg1);
	assertThat (result1, isEqualTo(expected1));
}

UnitTest(testCommentaryMultiLine) {
    string msg1 = "something\nelse\n happened";
    string expected1 = "# something\n# else\n#  happened";
    string result1 = CppUnitLite::UnitTest::msgComment(msg1);
	assertThat (result1, isEqualTo(expected1));
}

UnitTest(testCommentaryMixed) {
    string msg1 = "# something\nelse\n# happened";
    string expected1 = "# something\n# else\n# happened";
    string result1 = CppUnitLite::UnitTest::msgComment(msg1);
	assertThat (result1, isEqualTo(expected1));
}

UnitTest(testCommentaryCleanEnding) {
    string msg1 = "# something\nelse\n# happened\n";
    string expected1 = "# something\n# else\n# happened\n# ";
    string result1 = CppUnitLite::UnitTest::msgComment(msg1);
	assertThat (result1, isEqualTo(expected1));
}

UnitTest(testCommentaryDirtyEnding) {
    string msg1 = "# something\nelse\n# happened\nx";
    string expected1 = "# something\n# else\n# happened\n# x";
    string result1 = CppUnitLite::UnitTest::msgComment(msg1);
	assertThat (result1, isEqualTo(expected1));
}


UnitTest(testFailureMsg) {
    string msg1 = "# something\nelse\n# happened\nx";
    string expected1 = "# diag\nnot ok 42 - smallTest";
    string result1 = CppUnitLite::UnitTest::msgFailed(42, "smallTest", "diag", 499);
	assertThat (result1, isEqualTo(expected1));
}

