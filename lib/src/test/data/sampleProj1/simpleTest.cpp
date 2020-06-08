/**
 *  A very basic unit test.
 */


#include "unittest.h"

using namespace std;

UnitTest(SimpleTest) {
  assertThat(2, isEqualTo(1+1));
}
