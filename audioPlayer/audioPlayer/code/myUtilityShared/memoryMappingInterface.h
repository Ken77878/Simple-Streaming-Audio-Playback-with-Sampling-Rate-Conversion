#pragma once
#include "myUtilityDLLHeader.h"
#include <cstdint>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include "characterCodeMacro.h"
#include <string>
#include "noCopyAndMoveTemplate.h"

namespace MyUtility {
class myUtility_API MemoryMappingInterface
    : public NoCopyAndMove<MemoryMappingInterface> {
private:
#ifdef WIN32
  HANDLE fileHandle = nullptr;
  HANDLE mapHandle = nullptr;
#else
  std::int32_t mmapFd = 0;
#endif
  void *mmapDataTopPtr = nullptr;
  std::uint32_t mappedDataSize = 0;

public:
  std::uint16_t initialize(std::TSTRING &filePath);
  const void *getMmapDataTopPtr() const noexcept;
  std::uint16_t finalize();
  ~MemoryMappingInterface();
};
} // namespace MyUtility
