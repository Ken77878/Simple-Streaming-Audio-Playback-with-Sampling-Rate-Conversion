#pragma once
#include <vector>
#include <cstdint>
#include <cstdio>

std::uint16_t fmtChunk(FILE *fp, std::vector<std::uint8_t> &buffer,
                       uint8_t &channelSize, std::uint32_t &samplingRate);
std::uint16_t factChunk(FILE *fp, std::vector<std::uint8_t> &buffer);
std::uint16_t dataChunk(FILE *fp, std::vector<std::uint8_t> &buffer,
                        std::uint32_t &dataSize, std::uint32_t &dataOffset);
std::uint16_t listChunk(FILE *fp, std::vector<std::uint8_t> &buffer);
std::uint16_t checkWav(FILE *fp, std::uint32_t &dataSize,
                       std::uint32_t &dataOffset, std::uint8_t &channelSize,
                       std::uint32_t &samplingRate);
