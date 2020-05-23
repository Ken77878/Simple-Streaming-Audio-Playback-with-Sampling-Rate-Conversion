#include "audioTest1.h"
#include "audioInterface.h"
#include <cstdio>

const TCHAR* const audioFile1Path = TEXT(R"(/home/yourAudioFileName.wav)");
const TCHAR* const audioFile2Path = TEXT(R"(yourAudioFilePath)");

std::uint16_t audioTest1() {
  if (AUDIO_INTERFACE.initialize())
    return 1;

  std::uint32_t fileId1 = 0;
  printf("Input enter key.\n");
  getchar();
  if (AUDIO_INTERFACE.add(fileId1,
                    audioFile1Path    ,
                          true, DataReadingMethod::successive))
    return 1;
  if (AUDIO_INTERFACE.start(fileId1))
    return 1;
  printf("Input enter key.\n");
  getchar();

  std::uint32_t fileId2 = 0;

  if (AUDIO_INTERFACE.add(fileId2,
                       audioFile2Path  ,
                          true, DataReadingMethod::successive))
    return 1;

  if (AUDIO_INTERFACE.start(fileId2))
    return 1;
  printf("Input enter key.\n");
  getchar();
  if (AUDIO_INTERFACE.remove(fileId2))
    return 1;
  printf("Input enter key.\n");
  getchar();
  if (AUDIO_INTERFACE.stop(fileId1))
    return 1;
  printf("Input enter key.\n");
  getchar();
  if (AUDIO_INTERFACE.start(fileId1))
    return 1;
  printf("Input enter key.\n");
  getchar();

  if (AUDIO_INTERFACE.finalize())
    return 1;

  printf("program end Input enter key.\n");
  getchar();

  return 0;
}
