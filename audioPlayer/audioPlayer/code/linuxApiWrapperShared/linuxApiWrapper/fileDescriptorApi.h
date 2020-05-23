#pragma once
#include <cstdint>

namespace LinuxApiWrapper {

std::uint16_t writeWrap(std::uint32_t &returnByteSize, std::int32_t fd,
                        const void *bufferPtr, std::size_t byteCount);

std::uint16_t writeWrap(std::uint32_t &returnByteSize, std::int32_t fd,
                        const void *bufferPtr, std::size_t byteCount,
                        bool &eagainFlag);

std::uint16_t readWrap(std::uint32_t &returnByteSize, std::int32_t fd,
                       void *bufferPtr, std::size_t byteCount);

std::uint16_t readWrap(std::uint32_t &returnByteSize, std::int32_t fd,
                       void *bufferPtr, std::size_t byteCount,
                       bool &eagainFlag);
std::uint16_t closeWrap(std::int32_t &fileDiscriptor);

} // namespace LinuxApiWrapper
