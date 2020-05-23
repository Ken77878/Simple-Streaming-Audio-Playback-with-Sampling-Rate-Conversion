#include "greatestCommonDevisorCalculation.h"

std::uint32_t getGreatestCommonDevisorInRecursion(std::uint32_t a,
                                                  std::uint32_t b) {
  std::uint32_t r = a % b;
  if (r != 0)
    return getGreatestCommonDevisorInRecursion(b, r);
  else
    return b;
}
