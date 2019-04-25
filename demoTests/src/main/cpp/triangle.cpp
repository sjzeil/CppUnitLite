#include "triangle.h"
#include <cmath>

using namespace std;


  const std::string Triangle::ACUTE = "acute";
  const std::string Triangle::RIGHT = "right";
  const std::string Triangle::OBTUSE = "obtuse";
  const std::string Triangle::BAD = "not-a-triangle";



	
/**
 * Categorize a triangle based upon the length of its
 * sides. Output must be one of the four constant strings
 * listed above.
 * 
 * @param a length of one side
 * @param b length of another side
 * @param c length of a third side
 * @return string indicating the nature of the triangle
 */
std::string Triangle::categorizeTriangle (double a, double b, double c) {
  if (a < b) {
	double temp = a;
	a = b;
	b = temp;
  }
  if (a < c) {
	double temp = a;
	a = c;
	c = temp;
  }
  if (abs(a) > abs(b) + abs(c)) {
	return BAD;
  } else {
	double d = a*a - (b*b + c * c);
	if (d == 0.0) {
	  return RIGHT;
	} else if (d < 0.0) {
	  return ACUTE;
	} else {
	  return OBTUSE;
	}
  }
}


