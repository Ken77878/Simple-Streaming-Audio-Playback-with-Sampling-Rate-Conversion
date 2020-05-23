#pragma once
#include "myUtilityDLLHeader.h"
#include "characterCodeMacro.h"
#include <string>
#include <cstdint>

namespace MyUtility {
	myUtility_API std::uint16_t getBinaryFileSize(const std::TSTRING &filePath,
		std::uint32_t & fileSize);
}
