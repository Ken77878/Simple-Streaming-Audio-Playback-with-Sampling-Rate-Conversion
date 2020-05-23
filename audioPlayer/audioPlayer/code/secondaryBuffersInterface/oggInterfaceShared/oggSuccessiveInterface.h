#pragma once

#include "oggInterfaceDLLheader.h"
#include "oggInterfaceBase.h"
namespace SecondaryBuffer {
namespace Ogg {
class oggInterface_API SuccessiveInterface : public FileBase {
private:
  // in the middle of play
  bool continuance = false;
  std::uint32_t preExtraPcmDataFrameSize = 0;
  std::vector<float> preExtraPcmDataArray;
  std::uint16_t getPcmInt16bitData();
  std::uint16_t getPcmFloat32bitData();

  std::uint16_t playEndWithoutLoopAtFileEnd();
  std::uint16_t firstPageExist();
  std::uint16_t readOggPacketPart();
  std::uint16_t readOggDataPagePart();
  std::uint16_t readOggData();
  std::uint16_t oggDecodeTask();
  void setLoop(bool loopFlag);
  std::uint16_t initializeSamplingRateConversion();

public:
  SuccessiveInterface(std::uint8_t deviceChannelQuantity,
                      std::uint16_t deviceSamlingRate,
                      std::uint32_t framesPerPeriod,
                      DataReadingMethod dataReadingMethodFlag, bool loop);

  std::uint16_t initialize(std::TSTRING &filePath);
  std::uint16_t readDataFromFile();
  std::int16_t getDataFromVector(std::uint8_t channelIndex);
  std::int16_t getBufferData(std::uint8_t channelIndex);
  std::uint16_t finalize();
  ~SuccessiveInterface();
};

} // namespace Ogg
} // namespace SecondaryBuffer
