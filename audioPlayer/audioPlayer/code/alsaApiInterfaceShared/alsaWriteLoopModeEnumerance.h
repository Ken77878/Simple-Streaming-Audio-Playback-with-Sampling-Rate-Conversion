#pragma once

enum class AlsaWriteLoopMode {
  normal,
  mmap,
  alsaPoll,
  callbackEvent,
  callbackSignal
};
