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


// #define DEFAULT_UNIT_TEST_TIME_LIMIT -1L

UnitTest(testRelationals) {
	assertThat("abc", isEqualTo("abc"));
	assertThat(string("abc"), isEqualTo("abc"));
	assertThat(string("abc"), isEqualTo(string("abc")));
	assertThat(22, !isEqualTo(21));
	assertThat(21, is(21));
	assertThat(string("abc"), isNotEqualTo("abd"));
	assertThat(string("abc"), isNotEqualTo(string("bc")));
	assertThat(21, isNot(22));
	assertThat(21, isNotEqualTo(22));

	assertThat(1.001, isApproximately(1.0, 0.0011));
	assertThat(0.999, isApproximately(1.0, 0.0011));
	assertThat(1.01, !isApproximately(1.0, 0.0011));
	assertThat(0.99, !isApproximately(1.0, 0.0011));


	assertThat(21, isLessThan(22));
	assertThat(string("def"), isGreaterThan("abc"));
	assertThat(21, isLessThanOrEqualTo(22));
	assertThat(21, isLessThanOrEqualTo(21));
	assertThat(string("def"), isGreaterThanOrEqualTo("abc"));
	assertThat(string("def"), isGreaterThanOrEqualTo("def"));


    assertThat(23, isOneOf(1, 2, 23, 45));

}


UnitTest(testStringMatchers) {
	assertThat(string("abc"), contains("bc"));
	assertThat(string("abc"), !contains("bcd"));
	assertThat(string("abc"), endsWith("bc"));
	assertThat(string("abc"), !endsWith("ab"));
	assertThat(string("abc"), startsWith("ab"));
	assertThat(string("abc"), !startsWith("bc"));
}


UnitTest(testPointers) {
	int* p = nullptr;
	int q[1] = {1};
	assertThat(p, isNull());
	assertThat(q, isNotNull());
}




UnitTest(testContainers) {
	int numbers[] = {1, 3, 5, 9};
    vector<int> v  (numbers, numbers+4);
    list<int> L (numbers, numbers+4);

    assertThat(v, contains(3));
    assertThat(v, hasItem(3));
    assertThat(v, !hasItem(2));

    assertThat(L, hasItems(3, 9));
    assertThat(range(v.begin(), v.end()), hasItem(9));
    assertThat(arrayOfLength(numbers, 4), hasItem(3));


    assertThat(3, isIn(v));
    assertThat(3, isInRange(v.begin(), v.end()));

}


UnitTest(testAssocContainers) {
	int numbers[] = {1, 3, 5, 9};
    set<int> s  (numbers, numbers+4);
    map<int,int> m;
    m[1] = 2;
    m[3] = 6;
    m[5] = 10;

    assertThat(s, contains(3));
    assertThat(s, hasItem(3));
    assertThat(s, hasItems(3, 9));
    assertThat(3, isIn(s));
    assertThat(s, hasKey(3));
    assertThat(m, hasKey(3));
    assertThat(s, hasKeys(3, 5));
    assertThat(m, hasEntry(5, 10));

    assertThat(3, isIn(s));
}


UnitTest(testCombinations) {
	assertThat(23, allOf(isLessThan(42)));
	assertThat(23, allOf(isLessThan(42), isGreaterThan(10), is(23)));
	assertThat(23, !allOf(isLessThan(4), isGreaterThan(10), is(23)));
	assertThat(23, !allOf(isLessThan(42), isGreaterThan(100), is(23)));
	assertThat(23, !allOf(isLessThan(42), isGreaterThan(10), is(231)));

	assertThat(23, anyOf(isLessThan(42), isGreaterThan(10)));
	assertThat(23, anyOf(isLessThan(4), isGreaterThan(10)));
	assertThat(23, anyOf(isLessThan(42), isGreaterThan(100)));
	assertThat(23, !anyOf(isLessThan(2), isGreaterThan(100)));
}


