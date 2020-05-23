#include "primaryBufferInterface.h"
#include "wavSuccessiveInterface.h"
#include "wavCollectiveInterface.h"
#include "oggCollectiveInterface.h"
#include "oggSuccessiveInterface.h"
#include "audioMessageAdd.h"
#include <iostream>
#include <algorithm>
#include "debugMacro.h"

PrimaryBufferInterface::PrimaryBufferInterface() : playOn(false) {}

std::uint16_t
PrimaryBufferInterface::initialize(std::uint8_t deviceChannelQuantity,
                                   std::uint32_t deviceSamplingRate,
                                   std::uint32_t framesPerPeriod) {
  this->deviceChannelQuantity = deviceChannelQuantity;
  this->deviceSamplingRate = deviceSamplingRate;
  this->framesPerPeriod = framesPerPeriod;
  return 0;
}

std::uint16_t PrimaryBufferInterface::addMessageTask(
    std::uint32_t fileId,
    std::unique_ptr<SecondaryBuffer::CommonBase> &&secondaryBufferPtr) {
  DEBUG_PRINT_ARGS(TEXT("addMessageTask: fileId %u\n"), fileId);
  secondaryBufferArray[fileId] = std::move(secondaryBufferPtr);
  return 0;
}

std::uint16_t PrimaryBufferInterface::startMessageTask(std::uint32_t fileId) {
  DEBUG_PRINT_ARGS(TEXT("startMessageTask: fileId %u\n"), fileId);

  auto iterator = secondaryBufferArray.find(fileId);
  if (iterator == secondaryBufferArray.end()) {
    printf("programming error: audio bad file fileId\n");
    return 1;
  }
  (*iterator).second->setPlayOn(true);
  return 0;
}

std::uint16_t PrimaryBufferInterface::stopMessageTask(uint32_t fileId) {
  auto iterator = secondaryBufferArray.find(fileId);
  if (iterator == secondaryBufferArray.end()) {
    printf("programming error: audio bad file fileId\n");
    return 1;
  }
  (*iterator).second->setPlayOn(false);
  return 0;
}

std::uint16_t PrimaryBufferInterface::removeMessageTask(std::uint32_t fileId) {
  auto iterator = secondaryBufferArray.find(fileId);
  if (iterator == secondaryBufferArray.end()) {
    printf("programming error: audio bad file fileId\n");
    return 1;
  }
  if ((*iterator).second->finalize())
    return 1;
  iterator = secondaryBufferArray.erase(iterator);
  return 0;
}

std::uint16_t PrimaryBufferInterface::allRemoveMessageTask() {
  auto iterator = secondaryBufferArray.begin();
  for (;;) {
    if (iterator == secondaryBufferArray.end()) {
      break;
    }
    if ((*iterator++).second->finalize())
      return 1;
  }

  secondaryBufferArray.clear();
  return 0;
}

std::uint16_t PrimaryBufferInterface::currentAllMessageTask() {
  std::uint32_t messageQuantity = messageQueue.size();
  for (std::uint32_t u = 0; u < messageQuantity; ++u) {
    switch (messageQueue.front()->getMessageId()) {
    case AudioMessageId::add:
      if (addMessageTask(
              dynamic_cast<AudioMessage::OneFile *>(messageQueue.front().get())
                  ->getFileId(),
              reinterpret_cast<std::unique_ptr<AudioMessage::Add> &>(
                  messageQueue.front())
                  ->getSecondaryBufferUniquePtr()

                  ))
        return 1;
      break;
    case AudioMessageId::start:
      if (startMessageTask(
              dynamic_cast<AudioMessage::OneFile *>(messageQueue.front().get())
                  ->getFileId()))
        return 1;
      break;
    case AudioMessageId::stop:
      if (stopMessageTask(
              dynamic_cast<AudioMessage::OneFile *>(messageQueue.front().get())
                  ->getFileId()))
        return 1;
      break;
    case AudioMessageId::remove:
      if (removeMessageTask(
              dynamic_cast<AudioMessage::OneFile *>(messageQueue.front().get())
                  ->getFileId()))
        return 1;
      break;
    case AudioMessageId::allRemove:
      if (allRemoveMessageTask())
        return 1;
      break;
    }
    messageQueue.pop();
  }
  return 0;
}
void PrimaryBufferInterface::setPrimaryBufferPlayStatus() {
  playOn = false;
  for (auto &bufferIte : secondaryBufferArray) {
    if (bufferIte.second->getPlayOn()) {
      playOn = true;
      break;
    }
  }
  return;
}

static const TCHAR *checkFileExtension(const TCHAR *filePath) {
  return tstrrchr(filePath, TEXT('.'));
  // char * slashPtr = strchar(extensionStrPtr, '/');
}

std::uint16_t PrimaryBufferInterface::add(std::uint32_t &fileId,
                                          std::TSTRING &&filePath, bool loop,
                                          DataReadingMethod method) {
  if ((std::numeric_limits<std::uint32_t>::max)() == newFileId)
    newFileId = 1;
  else
    ++newFileId;
  fileId = newFileId;

  std::unique_ptr<SecondaryBuffer::CommonBase> secondaryBufferPtr;
  const TCHAR *extensionPt = checkFileExtension(filePath.c_str());
  if (!tstrcmp(extensionPt, TEXT(".wav")))
    switch (method) {
    case DataReadingMethod::successive:
      secondaryBufferPtr.reset(new SecondaryBuffer::Wav::SuccessiveInterface(
          deviceChannelQuantity, deviceSamplingRate, framesPerPeriod,
          DataReadingMethod::successive, loop));
      break;
    case DataReadingMethod::collective:
      secondaryBufferPtr.reset(new SecondaryBuffer::Wav::CollectiveInterface(
          deviceChannelQuantity, deviceSamplingRate, framesPerPeriod,
          DataReadingMethod::collective, loop));
      break;
    }
  else if (!tstrcmp(extensionPt, TEXT(".ogg")))
    switch (method) {
    case DataReadingMethod::successive:
      secondaryBufferPtr.reset(new SecondaryBuffer::Ogg::SuccessiveInterface(
          deviceChannelQuantity, deviceSamplingRate, framesPerPeriod,
          DataReadingMethod::successive, loop));
      break;
    case DataReadingMethod::collective:
      secondaryBufferPtr.reset(new SecondaryBuffer::Ogg::CollectiveInterface(
          deviceChannelQuantity, deviceSamplingRate, framesPerPeriod,
          DataReadingMethod::collective, loop));
      break;
    }
  if (secondaryBufferPtr->initialize(filePath))
    return 1;

  messageQueue.emplace(AudioMessageId::add, fileId,
                       std::move(secondaryBufferPtr));
  return 0;
}

std::uint16_t PrimaryBufferInterface::start(std::uint32_t fileId) {
  DEBUG_PRINT_ARGS(TEXT("messageQueue:start: fileId: %u\n"), fileId);
  messageQueue.emplace(AudioMessageId::start, fileId);
  return 0;
}
std::uint16_t PrimaryBufferInterface::stop(std::uint32_t fileId) {
  messageQueue.emplace(AudioMessageId::stop, fileId);
  return 0;
}
std::uint16_t PrimaryBufferInterface::remove(uint32_t fileId) {
  if (getPlayOn())
    messageQueue.emplace(AudioMessageId::remove, fileId);
  else {
    if (removeMessageTask(fileId))
      return 1;
  }
  return 0;
}
std::uint16_t PrimaryBufferInterface::allRemove() {
  if (getPlayOn())
    messageQueue.emplace(AudioMessageId::allRemove);
  else {
    if (allRemoveMessageTask())
      return 1;
  }
  return 0;
}
std::uint16_t PrimaryBufferInterface::readFileDataIfSuccessively() {
  for (auto &bufferIte : secondaryBufferArray)
    if (bufferIte.second->getDataReadingMethod() ==
        DataReadingMethod::successive) {
      //  printf("readFile, fileId: %u\n", bufferIte.first);
      bufferIte.second.get()->readDataFromFile();
    }
  return 0;
}

std::uint16_t
PrimaryBufferInterface::setSampleData(std::int16_t *out,
                                      std::uint8_t deviceChannelIndex) {

  *out = 0;
  std::int32_t primaryBufferData = 0;
  std::int16_t secondaryBufferData = 0;
  for (auto &bufferIte : secondaryBufferArray) {
    if (!bufferIte.second->getPlayOn())
      continue;
    secondaryBufferData = bufferIte.second->getBufferData(deviceChannelIndex);
    primaryBufferData += secondaryBufferData;
  }
  if (primaryBufferData > ((std::numeric_limits<int16_t>::max)())) {
    *out = (std::numeric_limits<int16_t>::max)();

    DEBUG_PRINT(TEXT("audio noise(plus) occurred.\n"));
  } else if (primaryBufferData < ((std::numeric_limits<int16_t>::min)())) {
    *out = (std::numeric_limits<int16_t>::min)();
    DEBUG_PRINT(TEXT("audio noise(minus) occurred.\n"));
  } else
    *out = static_cast<std::int16_t>(primaryBufferData);

  return 0;
}

std::uint16_t
PrimaryBufferInterface::makeAllPrimaryBufferData(std::int16_t *audioBufferPtr) {

  if (readFileDataIfSuccessively())
    return 1;
  std::int16_t *out = audioBufferPtr;
  for (uint32_t u = 0; u < framesPerPeriod; ++u)
    for (std::uint32_t v = 0; v < deviceChannelQuantity; ++v) {
      setSampleData(out++, v);
    }
  return 0;
}

/*std::uint16_t PrimaryBufferInterface::makePartPrimaryBufferDataPreparation() {
  if (readFileDataIfSuccessively())
    return 1;
  return 0;
}
std::uint16_t
PrimaryBufferInterface::makePartPrimaryBufferData(std::int16_t *alsaMmapPtr,
                                                  std::uint32_t frameSize) {
  DEBUG_PRINT(TEXT("makePartPrimaryBufferData start\n"));

  for (uint32_t u = 0; u < frameSize; ++u)
    for (std::uint32_t v = 0; v < deviceChannelQuantity; ++v) {
      setSampleData(alsaMmapPtr++, v);
    }
  return 0;
}*/

/*void PrimaryBufferInterface::setRestartFlag(bool flag) noexcept {
  restartFlag = flag;
}
bool PrimaryBufferInterface::getRestartFlag() const noexcept {
  return restartFlag;
}*/

bool PrimaryBufferInterface::getPlayOn() const noexcept { return playOn; }
void PrimaryBufferInterface::setPlayOn(bool flag) noexcept {
#ifdef _DEBUG
  if (flag)
    printf("=====primaryBuffer play on=========\n");
  else
    printf("=====primaryBuffer play off=========\n");
#endif
  playOn = flag;
}
