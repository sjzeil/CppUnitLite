#include "triangle.h"
#include <iostream>

using namespace std;

/**
 * @param args three floating point numbers
 */
int main(int argc, char** argv) {
  if (argc != 4) {
	cerr << "Usage: " << argv[0] << " len1 len2 len3" <<endl;
	return -1;
  }

  double a = stod(argv[1]);
  double b = stod(argv[2]);
  double c = stod(argv[3]);
  string result = Triangle::categorizeTriangle(a, b, c);
  cout << result << endl;
  return 0;
}
