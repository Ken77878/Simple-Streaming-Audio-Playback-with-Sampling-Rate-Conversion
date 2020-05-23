#include "checkWav.h"
#include "debugMacro.h"
#include "fileManagement.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

std::uint16_t fmtChunk(FILE *fp, std::vector<std::uint8_t> &buffer,
                       uint8_t &channelSize, std::uint32_t &samplingRate) {

  /*struct FormatChunk {24byte
    uint32_t id; 4 big
    int32_t size; 4 little
    int16_t format; 2 little
    uint16_t channels; 2 little
    uint32_t samplerate; 4 little
    uint32_t bytepersec; 4little
    uint16_t blockalign; 2little
    uint16_t bitswidth; 2 little
};*/

  if (MyUtility::fseekWrap(fp, 4, SEEK_CUR))
    return 1;

  // check if this is linear PCM
  buffer.resize(2);
  if (MyUtility::freadWrap(&buffer[0], 1, 2, fp))
    return 1;
  if (*(reinterpret_cast<uint16_t *>(&buffer[0])) != 1) {
    DEBUG_PRINT_ARGS(TEXT("type of format: %u\n"),
                     *(reinterpret_cast<uint16_t *>(&buffer[0])));
    std::fprintf(stderr, "This isn't linear PCM.\n");
    return 1;
  }
  if (MyUtility::freadWrap(&buffer[0], 1, 2, fp))
    return 1;
  channelSize = *(reinterpret_cast<uint16_t *>(&buffer[0]));
  DEBUG_PRINT_ARGS(TEXT("channelSize: %u\n"), channelSize);

  buffer.resize(4);
  if (MyUtility::freadWrap(&buffer[0], 1, 4, fp))
    return 1;
  samplingRate = *(reinterpret_cast<uint32_t *>(&buffer[0]));
  DEBUG_PRINT_ARGS(TEXT("sampling rate: %u\n"), samplingRate);

  if (MyUtility::fseekWrap(fp, 6, SEEK_CUR))
    return 1;
  buffer.resize(2);
  if (MyUtility::freadWrap(&buffer[0], 1, 2, fp))
    return 1;
  if (*(reinterpret_cast<uint16_t *>(&buffer[0])) != 16) {
    std::fprintf(stderr, "wav error: bit per sample must be 16.\n");
    return 1;
  }
  return 0;
}

std::uint16_t factChunk(FILE *fp, std::vector<std::uint8_t> &buffer) {
  buffer.resize(4);
  if (MyUtility::freadWrap(&buffer[0], 1, 4, fp))
    return 1;
  uint32_t chunkSize = *(reinterpret_cast<uint32_t *>(&buffer[0]));
  if (MyUtility::fseekWrap(fp, chunkSize, SEEK_CUR))
    return 1;
  return 0;
}

std::uint16_t dataChunk(FILE *fp, std::vector<std::uint8_t> &buffer,
                        uint32_t &dataSize, uint32_t &dataOffset) {
  buffer.resize(4);
  if (MyUtility::freadWrap(&buffer[0], 1, 4, fp))
    return 1;
  dataSize = *(reinterpret_cast<uint32_t *>(&buffer[0]));
  DEBUG_PRINT_ARGS(TEXT("dataSize: %u\n"), dataSize);

  std::int32_t offset = 0;
  if (MyUtility::ftellWrap(fp, offset))
    return 1;
  dataOffset = offset;
  DEBUG_PRINT_ARGS(TEXT("dataOffset: %u\n"), dataOffset);
  return 0;
}

std::uint16_t listChunk(FILE *fp, std::vector<std::uint8_t> &buffer) {
  buffer.resize(4);
  if (MyUtility::freadWrap(&buffer[0], 1, 4, fp))
    return 1;
  uint32_t chunkSize = *(reinterpret_cast<uint32_t *>(&buffer[0]));
  if (MyUtility::fseekWrap(fp, chunkSize, SEEK_CUR))
    return 1;
  return 0;
}

std::uint16_t checkWav(FILE *fp, uint32_t &dataSize, uint32_t &dataOffset,
                       uint8_t &channelSize, std::uint32_t &samplingRate) {
  // array<uint32_t, 3> dataInfo;// size, ofset, channels
  /*struct RiffHeader {12byte
          uint32_t riff; big
          int32_t  size; little
          uint32_t type; big
  };*/

  DEBUG_PRINT(TEXT("----- RiffHeader ------\n"));
  std::vector<std::uint8_t> buffer(4);

  if (MyUtility::freadWrap(&buffer[0], 1, 4, fp))
    return 1;
  if (buffer[0] != 'R' || buffer[1] != 'I' || buffer[2] != 'F' ||
      buffer[3] != 'F') {
    std::fprintf(stderr, "RIFF: This isn't wav file. \n");
    return 1;
  }

  if (fseek(fp, 4, SEEK_CUR))
    return 1;

  buffer.resize(4);
  if (MyUtility::freadWrap(&buffer[0], 1, 4, fp))
    return 1;
  if (buffer[0] != 'W' || buffer[1] != 'A' || buffer[2] != 'V' ||
      buffer[3] != 'E') {
    std::fprintf(stderr, "WAVE: This isn't wav file. \n");
    return 1;
  }

  for (;;) {
    buffer.resize(4);
    if (MyUtility::freadWrap(&buffer[0], 4, 1, fp))
      return 1;
    if (buffer[0] == 'f' && buffer[1] == 'm' && buffer[2] == 't' &&
        buffer[3] == ' ') {
      DEBUG_PRINT(TEXT("-----fmt chunk------\n"));
      if (fmtChunk(fp, buffer, channelSize, samplingRate))
        return 1;
    } else if (buffer[0] == 'f' && buffer[1] == 'a' && buffer[2] == 'c' &&
               buffer[3] == 't') {
      DEBUG_PRINT(TEXT("-----fact chunk------\n"));

      if (factChunk(fp, buffer))
        return 1;
    } else if (buffer[0] == 'd' && buffer[1] == 'a' && buffer[2] == 't' &&
               buffer[3] == 'a') {
      DEBUG_PRINT(TEXT("-----data chunk------\n"));

      if (dataChunk(fp, buffer, dataSize, dataOffset))
        return 1;
      break;
    } else if (buffer[0] == 'L' && buffer[1] == 'I' && buffer[2] == 'S' &&
               buffer[3] == 'T') {
      DEBUG_PRINT(TEXT("-----LIST chunk------\n"));

      if (listChunk(fp, buffer))
        return 1;
    } else {
      DEBUG_PRINT_ARGS(TEXT("%c"), buffer[0]);
      DEBUG_PRINT_ARGS(TEXT("%c"), buffer[1]);
      DEBUG_PRINT_ARGS(TEXT("%c"), buffer[2]);
      DEBUG_PRINT_ARGS(TEXT("%c"), buffer[3]);
      return 1;
    }
  }
  return 0;
}
