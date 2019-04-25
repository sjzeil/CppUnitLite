#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <string>

/**
 * @author zeil
 *
 */
class Triangle {
public:
  static const std::string ACUTE;
  static const std::string RIGHT;
  static const std::string OBTUSE;
  static const std::string BAD;
  
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
  static std::string categorizeTriangle (double a, double b, double c);
};

#endif
