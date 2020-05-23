#pragma once

#include "oggInterfaceDLLheader.h"
#include "oggInterfaceBase.h"
namespace SecondaryBuffer {
namespace Ogg {
class oggInterface_API CollectiveInterface : public FileBase {
private:
  bool fileEnd = false;
  std::uint32_t postPcmDataFrameSize = 0;
  std::uint16_t firstPageExist();
  std::uint16_t getPcmInt16bitData();
  std::uint16_t getPcmFloat32bitData();

  std::uint16_t readOggPacketPart();
  std::uint16_t readOggDataPagePart();
  std::uint16_t readOggData();
  std::uint16_t
  initializeSamplingRateConversion(std::uint32_t originalFrameSize);

  std::uint16_t oggDecodeTask();

  std::int16_t getDataFromVector(std::uint8_t channelIndex);

  std::uint16_t finalizeOggDecode();

public:
  CollectiveInterface(std::uint8_t deviceChannelQuantity,
                      std::uint16_t deviceSamlingRate,
                      std::uint32_t framesPerPeriod,
                      DataReadingMethod dataReadingMethodFlag, bool loop);

  std::uint16_t initialize(std::TSTRING &filePath);

  std::int16_t getBufferData(std::uint8_t channelIndex);
  std::uint16_t finalize();
  ~CollectiveInterface();
};
} // namespace Ogg
} // namespace SecondaryBuffer
