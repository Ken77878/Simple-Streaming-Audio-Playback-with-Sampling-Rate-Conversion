#pragma once
#include "secondaryBufferBaseDLLheader.h"
#include "samplingRateConvertorInterface.h"
#include "dataReadingMethod.h"
#include "characterCodeMacro.h"
#include "noCopyAndMoveTemplate.h"

namespace SecondaryBuffer {
class secondaryBufferBase_API CommonBase : public NoCopyAndMove<CommonBase> {
private:
protected:
  std::uint8_t deviceChannelQuantity = 0;
  std::uint8_t fileChannelQuantity = 0;
  std::uint32_t framesPerPeriod = 0;
  std::uint32_t preSamplingRate = 0;
  std::uint32_t postSamplingRate = 0;
  DataReadingMethod dataReadingMethodFlag;
  bool playOn = false;
  bool loop = false;
  std::uint32_t currentReadFrameIndex = 0;
  SamplingRate::ConvertorInterface samplingRateConvertor;

public:
  CommonBase(std::uint8_t deviceChannelQuantity,
             std::uint16_t deviceSamlingRate, std::uint32_t framesPerPeriod,
             DataReadingMethod dataReadingMethodFlag, bool loop);
  virtual std::uint16_t initialize(std::TSTRING &filePath) = 0;
  virtual std::uint16_t readDataFromFile();
  virtual std::int16_t getBufferData(std::uint8_t channelIndex) = 0;

  bool getPlayOn() const noexcept;
  void setPlayOn(bool playOnFlag) noexcept;
  DataReadingMethod getDataReadingMethod() const noexcept;
  virtual std::uint16_t finalize() = 0;
  virtual ~CommonBase() {}
};

} // namespace SecondaryBuffer
