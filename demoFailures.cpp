/*
 * demoFailures.cpp
 *
 *  Demo of failure messages produced by the CppUnitLite assertions.
 *
 *  Every test in this demo will fail, allowing a demo of what diagnostics
 *  are offered upon test failure.
 *
 *  Created on: Jul 6, 2018
 *      Author: zeil
 */

/**
 *  Unit test of the unit test framework
 */

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>

#include "unittest.h"

using namespace std;


vector<int> v1 {1, 2, 3};
vector<int> v1b {1, 2, 3};
vector<int> v2 {2, 4, 6};
string a1 = "abc";
string a2 = "def";

// #define DEFAULT_UNIT_TEST_TIME_LIMIT -1L


UnitTest(assertTrueFails) {
	assertTrue(v1.empty());
}

UnitTest(assertFalseFails) {
	assertFalse(vector<int>().empty());
}

UnitTest(assertEqualsFails) {
	assertEqual(v1, v2);
}

UnitTest(assertNotEqualsFails) {
	assertNotEqual(v1, v1b);
}

UnitTest(assertNullFails) {
	assertNull(a1.c_str());
}

UnitTest(assertNotNullFails) {
	assertNotNull(nullptr);
}

UnitTest(isEqualToFails) {
	assertThat(a1, isEqualTo(a2));
}

UnitTest(isNotEqualToFails) {
	assertThat(v1, isNotEqualTo(v1b));
}

UnitTest(notIsEqualToFails) {
	assertThat(v1, !isEqualTo(v1b));
}

UnitTest(isFails) {
	assertThat(a1, is(a2));
}

UnitTest(isNotFails) {
	assertThat(v1, isNot(v1b));
}

UnitTest(notIsFails) {
	assertThat(v1, !is(v1b));
}

UnitTest(isApproxFails) {
	double x = 1.2;
	double y = 1.0;
	assertThat(x, isApproximately(y, 0.1));
}

UnitTest(lessThanFails) {
	double x = 1.2;
	double y = 1.0;
	assertThat(x, isLessThan(y));
}

UnitTest(lessThanOrEqFails) {
	double x = 1.2;
	double y = 1.0;
	assertThat(x, isLessThanOrEqualTo(y));
}

UnitTest(greaterThanFails) {
	double x = 1.2;
	double y = 1.0;
	assertThat(y, isGreaterThan(y));
}

UnitTest(greaterThanOrEqFails) {
	double x = 1.2;
	double y = 1.0;
	assertThat(y, isGreaterThanOrEqualTo(x));
}


UnitTest(isOneOfFails) {
	assertThat (string("abcd"), isOneOf(a1, a2));
}

UnitTest(stringContainsFails) {
	assertThat(a1, contains(a2));
}

UnitTest(notStringContainsFails) {
	assertThat(a1, !contains(a1));
}

UnitTest(stringEndsWithFails) {
	assertThat(a1, endsWith(a2));
}

UnitTest(stringStartsWithFails) {
	assertThat(a1, startsWith(a2));
}

UnitTest(isNullFails) {
	assertThat(a1.c_str(), isNull());
}

UnitTest(isNotNullFails) {
	assertThat(nullptr, isNotNull());
}


UnitTest (containsFails) {
	assertThat(v1, contains(42));
}

UnitTest (hasItemFails) {
	assertThat(v1, hasItem(42));
}

UnitTest (rangeHasItemFails) {
	assertThat(range(v1.begin(), v1.end()), hasItem(42));
}

UnitTest (notHasItemFails) {
	assertThat(v1, !hasItem(v1[1]));
}

UnitTest (hasItemsFails) {
	assertThat(v1, hasItems(2, 3, 42));
}

UnitTest(arrayHasItemFails) {
	int numbers[] = {1, 3, 5, 9};
	assertThat(arrayOfLength(numbers, 3), hasItem(9));
}

UnitTest(matchesFails) {
	int numbers[] = {1, 3, 5, 9};
	assertThat(arrayOfLength(numbers, 3), matches(range(v2.begin(), v2.end())));
}

UnitTest(isInFails) {
	assertThat (42, isIn(v1));
}

UnitTest(isInRangeFails) {
	assertThat (42, isInRange(v1.begin(), v1.end()));
}


UnitTest(hasKeyFails) {
    map<int,string> m;
    m[1] = "abc";
    m[3] = "def";
    m[5] = "ghi";
	assertThat(m, hasKey(2));
}

UnitTest(hasKeysFails) {
    map<string, int> m;
    m["abc"] = 1;
    m["def"] = 3;
    m["ghi"] = 5;;
	assertThat(m, hasKeys(string("abc"), string("qrs"), string("def")));
}

UnitTest(hasEntryFails) {
    map<int,string> m;
    m[1] = "abc";
    m[3] = "def";
    m[5] = "ghi";
	assertThat(m, hasEntry(4, string("abcdef")));
}

UnitTest(hasEntry2Fails) {
    map<int,string> m;
    m[1] = "abc";
    m[3] = "def";
    m[5] = "ghi";
	assertThat(m, hasEntry(3, string("abcdef")));
}



UnitTest(allOfFails) {
	assertThat (23, allOf(isLessThan(42), isGreaterThan(25)));
}

UnitTest(notAllOfFails) {
	assertThat (23, !allOf(isLessThan(42), isGreaterThan(20)));
}

UnitTest(anyOfFails) {
	assertThat (23, anyOf(isLessThan(2), isGreaterThan(25)));
}

UnitTest(notAnyOfFails) {
	assertThat (23, !anyOf(isLessThan(25), isGreaterThan(25)));
}



