#pragma once
#include "myUtilityDLLHeader.h"
#include <string>
#include "characterCodeMacro.h"

namespace MyUtility {
myUtility_API std::uint16_t fopenWrap(FILE *&fp, std::TSTRING &filePath);
myUtility_API std::uint16_t fseekWrap(FILE *stream, long offset, int origin);
myUtility_API std::uint16_t ftellWrap(FILE *stream, std::int32_t &offset);
myUtility_API   std::uint16_t freadWrap(void *bufferPtr, size_t byteSize, size_t count,
                        FILE *stream);
 myUtility_API std::uint16_t fcloseWrap(FILE *&fp);
} // namespace MyUtility
