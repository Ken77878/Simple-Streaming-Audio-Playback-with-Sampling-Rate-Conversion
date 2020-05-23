#include "memoryMappingInterface.h"
#include "systemErrorInterface.h"
#ifdef WIN32
#include "Win32ApiWrapper/handleApi.h"
#else
// mmap
#include <sys/mman.h>
// open
#include <fcntl.h>
// fstat
#include <sys/types.h>
#include <sys/stat.h>

#include "fileDescriptorManagement.h"
#include "linuxApiWrapper/fileDescriptorApi.h"
#endif

namespace MyUtility {
	std::uint16_t MemoryMappingInterface::initialize(std::TSTRING &filePath) {
#ifdef WIN32
		fileHandle = CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fileHandle == INVALID_HANDLE_VALUE) {
			SystemErrorInterface::systemErrorHandlingTask(TEXT("CreateFile"));
			return 1;
		}

		DWORD mappedDataSize = GetFileSize(fileHandle, NULL);
#ifdef _DEBUG
		printf("mappedDataSize: %u\n", mappedDataSize);
#endif
		mapHandle =
			CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, mappedDataSize, NULL);
		if (mapHandle == NULL) {
			fprintf(stderr, "errorCode:%d\n", GetLastError());
			return 1;
		}

		if ((mmapDataTopPtr = MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0,
			mappedDataSize)) == nullptr) {
			SystemErrorInterface::systemErrorHandlingTask(TEXT("MapViewOfFile"));
			return 1;
		}
#else
		std::int32_t mappedDataSizeFd = open(filePath.c_str(), O_RDONLY);
		if (mappedDataSizeFd == -1) {
			printf("failed to open wav file: %s\n", filePath.c_str());
			// strerror
			return 1;
		}
		FILE *streamForSize = nullptr;
		struct stat fileInfo = { 0 };
		if ((streamForSize = fdopen(mappedDataSizeFd, "rb")) == NULL) {
			SystemErrorInterface::systemErrorHandlingTask(TEXT("fdopen"));
			return 1;
		}
		if (fstat(mappedDataSizeFd, &fileInfo) == -1) {
			SystemErrorInterface::systemErrorHandlingTask(TEXT("fstat"));
			return 1;
		}

		mappedDataSize = fileInfo.st_size;
		// mappedDataSizeFd is closed by fclose automatically.
		if (fclose(streamForSize) != 0) {
			return 1;
		}

		mmapFd = open(filePath.c_str(), O_RDONLY);
		if (mmapFd == -1) {
			printf("failed to open wav file: %s\n", filePath.c_str());
			SystemErrorInterface::systemErrorHandlingTask(TEXT("open"));
			return 1;
		}

		if ((mmapDataTopPtr = reinterpret_cast<std::int16_t *>(
			mmap(NULL, mappedDataSize, PROT_READ, MAP_PRIVATE, mmapFd, 0))) ==
			MAP_FAILED) {
			SystemErrorInterface::systemErrorHandlingTask(TEXT("mmap"));
			return 1;
		}
#endif
		return 0;
	}

	const void *MemoryMappingInterface::getMmapDataTopPtr() const noexcept {
		return mmapDataTopPtr;
	}

	std::uint16_t MemoryMappingInterface::finalize() {
#ifdef WIN32
		if (mmapDataTopPtr) {
			if (!UnmapViewOfFile(mmapDataTopPtr)) {
				SystemErrorInterface::systemErrorHandlingTask(TEXT("UnmapViewOfFile"));
				return 1;
			}
			mmapDataTopPtr = nullptr;
		}
		if (Win32ApiWrapper::CloseHandleWrap(mapHandle)) {
			return 1;
		}

		if (Win32ApiWrapper::CloseHandleWrap(fileHandle)) {
			return 1;
		}
#else
		if (LinuxApiWrapper::closeWrap(mmapFd))
			return 1;
		if (mmapDataTopPtr) {
			if (munmap(mmapDataTopPtr, mappedDataSize) == -1) {
				printf("mnumap error\n");
				SystemErrorInterface::systemErrorHandlingTask(TEXT("munmap"));
				return 1;
			}
			mmapDataTopPtr = nullptr;
		}
#endif
		return 0;
	}

	MemoryMappingInterface::~MemoryMappingInterface() {
#ifdef _DEBUG
#ifdef WIN32
		if (fileHandle || mapHandle || mmapDataTopPtr)
#else
		if (mmapFd || mmapDataTopPtr)
#endif
		{
			printf("MemoryMappingInterface: memory leak\n");
			exit(555);
		}
#endif
	}

} // namespace MyUtility
