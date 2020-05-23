#pragma once
#include "endianInfo.h"
#include <iostream>
#include <cstdint>

inline Endian checkEndian() {
  std::uint16_t value = 256; // big: 0000000100000000, little:0000000000000001
  std::uint8_t *ptr = reinterpret_cast<std::uint8_t *>(&value);
  if (*ptr == 0) {
    std::cout << "This system is little endian." << std::endl;
    return Endian::little;
  } else {
    std::cout << "This system is big endian." << std::endl;
    return Endian::big;
  }
}
