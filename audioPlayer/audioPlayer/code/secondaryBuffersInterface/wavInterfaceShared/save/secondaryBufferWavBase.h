#pragma once
#include <cstdint>
#include "endianInfo.h"
#include <vector>
#include <cstdio>

#include "endianInfo.h"
namespace SecondaryBuffer {
class WavBase {
protected:
  Endian endianFlag;
  std::uint32_t dataOffset;
  std::uint32_t dataSize;
  WavBase(Endian endianFlag);
  std::uint16_t initialize(FILE *fp, Endian endianFlag, std::uint32_t &dataSize,
                           std::uint32_t &dataOffset, std::uint8_t &channels,
                           std::uint16_t &samplingRate);
  std::uint16_t freadWrap(FILE *fp, std::vector<std::int16_t> &data,
                          std::uint32_t size, std::uint32_t index = 0);
};
} // namespace SecondaryBuffer
