/**
 *  Unit test of the unit test framework
 */

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <tuple>
#include <utility>

#include "unittest.h"

using namespace std;


// #define DEFAULT_UNIT_TEST_TIME_LIMIT -1L

UnitTest(testPrimitivesRepr) {
	assertThat(CppUnitLite::getStringRepr(42), is("42"));
	assertThat(CppUnitLite::getStringRepr(1.23), is("1.23"));
	assertThat(CppUnitLite::getStringRepr('c'), is("'c'"));
	assertThat(CppUnitLite::getStringRepr(true), is("true"));
	assertThat(CppUnitLite::getStringRepr(false), is("false"));
}

class Foo {
	int i;
};
UnitTest(testUnprintableRepr) {
	Foo foo;
	assertThat(CppUnitLite::getStringRepr(foo), is("???"));
}

UnitTest(testIterableRepr) {
	array<int,3> a1 = {1, 2, 3};
	assertThat(CppUnitLite::getStringRepr(a1), is("[1, 2, 3]"));
	vector<char> v2 = {'a'};
	assertThat(CppUnitLite::getStringRepr(v2), is("['a']"));
	vector<char> v3;
	assertThat(CppUnitLite::getStringRepr(v3), is("[]"));
}

UnitTest(testMapRepr) {
	map<int,int> m;
	m[1] = 10;
	m[2] = 20;
	assertThat(CppUnitLite::getStringRepr(m), is("[<1, 10>, <2, 20>]"));
}


UnitTest(testTupleRepr) {
	auto t1 = make_tuple(42, true, 'a');
	assertThat(CppUnitLite::getStringRepr(t1), is("<42, true, 'a'>"));
	auto t2 = make_tuple(1.2);
	assertThat(CppUnitLite::getStringRepr(t2), is("<1.2>"));
	auto t3 = make_tuple();
	assertThat(CppUnitLite::getStringRepr(t3), is("<>"));
}

UnitTest(testPairRepr) {
	auto t1 = make_pair(42, true);
	assertThat(CppUnitLite::getStringRepr(t1), is("<42, true>"));
}


UnitTest(testStringRepr) {
	string s("xyz");
	assertThat(CppUnitLite::getStringRepr(s), is("\"xyz\""));
	assertThat(CppUnitLite::getStringRepr("def"), is("\"def\""));
}
