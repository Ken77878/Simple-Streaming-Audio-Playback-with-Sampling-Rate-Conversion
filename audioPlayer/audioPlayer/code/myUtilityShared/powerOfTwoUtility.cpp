#include "powerOfTwoUtility.h"
namespace MyUtility {
std::uint32_t getNearestBiggerPowerOfTwo(std::uint32_t number) {
  std::uint32_t powerOfTwo = 0;
  for (;;) {
    ++powerOfTwo;
    if (!(number >>= 1))
      break;
  }
  return (1 << powerOfTwo);
}
} // namespace MyUtility
