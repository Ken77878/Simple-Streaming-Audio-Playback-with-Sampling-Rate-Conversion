#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "dataReadingMethod.h"

namespace SecondaryBuffer {
class CommonBase {
protected:
  DataReadingMethod dataReadingMethodFlag;
  bool playOn = false;
  bool loop = false;
  std::uint8_t channels = 0;
  std::uint16_t usedSamplingRate = 44100;
  std::uint16_t samplingRate = 0;
  std::uint32_t framesPerPeriod = 0;

  std::uint16_t fopenWrap(FILE *&fp, std::string &&filePath);

public:
  CommonBase(DataReadingMethod dataReadingMethodFlag, bool loop,
             std::uint32_t framesPerPeriod);
  //  CommonBase();
  virtual ~CommonBase(){};
  virtual std::uint16_t initialize(std::string &&filePath) = 0;
  virtual std::uint16_t finalize() = 0;
  virtual std::int16_t getBufferData() = 0;
  virtual void updateStatus() = 0;
  DataReadingMethod getDataReadingMethod() const;
  void setPlayOn(bool flag);
  bool getPlayOn() const;
};
} // namespace SecondaryBuffer
