
#include "unittest.h"

#include "triangle.h"


using namespace std;

/**
 * Unit test for class Project.
 *
 * Uses test CppUnitLite framework from
 * https://git-community.cs.odu.edu/zeil/CppUnitLite
 */



UnitTest (functionalTests)
{
	string result = Triangle::categorizeTriangle(3.0, 4.0, 5.0);
	assertThat (result, is(Triangle::RIGHT));

	result = Triangle::categorizeTriangle(10.0, 10.0, 10.0);
	assertThat (result, is(Triangle::ACUTE));

	result = Triangle::categorizeTriangle(10.0, 10.0, 18.0);
	assertThat (result, is(Triangle::OBTUSE));

	result = Triangle::categorizeTriangle(10.0, 10.0, 100.0);
	// many ways to say almost the same thing
	assertTrue(result == Triangle::BAD);
	assertEqual(Triangle::BAD, result);
	assertThat(result, isEqualTo(Triangle::BAD));
	assertThat(result, contains(Triangle::BAD));
	assertThat(result, startsWith(Triangle::BAD));

}


UnitTest (boundaryTests)
{
	string result = Triangle::categorizeTriangle(3.0, 4.0, 7.0);
	assertThat (result, is(Triangle::OBTUSE));

	result = Triangle::categorizeTriangle(3.0, 4.0, 7.001);
	assertThat (result, is(Triangle::BAD));

}


