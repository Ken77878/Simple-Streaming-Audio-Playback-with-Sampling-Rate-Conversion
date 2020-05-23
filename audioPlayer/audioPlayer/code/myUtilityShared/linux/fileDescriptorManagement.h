#include <cstdint>
namespace MyUtility {

std::uint16_t writeAll(std::int32_t fd, const void *bufferPtr,
                       std::uint32_t &remainByteSize);
std::uint16_t readAll(std::int32_t fd, void *bufferPtr,
                      std::uint32_t &remainByteSize);
std::uint16_t setFileDescriptorNonblocking(std::int32_t fileDiscriptor);
} // namespace MyUtility
