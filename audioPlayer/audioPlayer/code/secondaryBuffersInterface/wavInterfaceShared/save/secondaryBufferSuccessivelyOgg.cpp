#include "secondaryBufferSuccessivelyOgg.h"
#include "audioError.h"
#include "portAudioConstants.h"

	void SecondaryBufferSuccessivelyOgg::initialize() {
		printf("SecondaryBufferSuccessivelyOgg::initialize()\n");
		/*std::thread t([&]() {

			oggVorbisInstance->oggTask();
			fileIndex = ++fileIndex % 2;
			ready = true;
			//SecondaryBuffer::start();
		});
		t.detach();*/

		loopThread.reset(new std::thread(&SecondaryBufferSuccessivelyOgg::threadTask, this));
	}

	void SecondaryBufferSuccessivelyOgg::threadTask() {
		oggVorbisInstance->oggTask();
	}

	void SecondaryBufferSuccessivelyOgg::startLoopThread() {
		//loopThread.reset(new std::thread(&SecondaryBufferSuccessivelyOgg::std::threadTask, this));
		//t.detach();

	}

	void SecondaryBufferSuccessivelyOgg::stop() {
		if (loopThread)
			loopThread->join();
	}


	SecondaryBufferSuccessivelyOgg::SecondaryBufferSuccessivelyOgg(ReadingMethod method, Endian endianFlag, std::TSTRING fileName,
		PaStream * stream, bool loop, std::mutex* startStreamMutexPt) :SecondaryBuffer(method, endianFlag, fileName, stream, loop, startStreamMutexPt)
		, lastBuffer(false),
		fileIndex(0),
		currentOffset(0) {

		errno_t error = tfopen_s(&fp, filePath.c_str(), TEXT("rb"));

		if (error != 0) {
			tprintf(TEXT("ファイルを開けませんでした: %s\n"), filePath.c_str());
			throw std::exception();
		}


		/*for (auto &data : dataArray)
			data.resize(::framesPerBuffer * 2);*/

		//bufferSize = sampleSize * channels * ::framesPerBuffer;

		/*if (dataSize < bufferSize) {
		printf("dataSize is too small.");
		system("pause");
		exit(1);
		}*/

		for (auto& event : events)
			event = CreateEventA(NULL, FALSE, FALSE, NULL);
		oggVorbisInstance.reset(new OggVorbisSuccessively(fp, loop, channels, dataArray, fileIndex,ready,  play, remove, events));
		initialize();
	}
	SecondaryBufferSuccessivelyOgg::~SecondaryBufferSuccessivelyOgg() {
		if (loopThread)
			loopThread->join();
		fclose(fp);
		for (auto& event : events) {
			if (CloseHandle(event) == 0)
			{
				printf("failed to close event.");
				AudioError::exit(1);
			}
		}

	}

	void SecondaryBufferSuccessivelyOgg::updateStatus() {
		fileIndex = ++fileIndex % 2;
	}

	int16_t SecondaryBufferSuccessivelyOgg::getBufferData(DataChannel channel, uint32_t frame) {
	//	static int32_t count = 0;
			
		uint32_t index = (fileIndex + 1) % 2;
		if (channels == 2) {
			unsigned char channelValue = 0;
			if (channel == DataChannel::right) {
				channelValue = 1;
			}
			//printf("dataArray index: %d", dataArray[index][frame * 2 + channelValue]);
		//	printf("index: %d\n", index);
		/*	for (int i = 0; i < 10; ++i) {
				printf("index %d: %d\n", 0, dataArray[0][i]);
				printf("index %d: %d\n", 1, dataArray[1][i]);
			}*/
		/*	if (count == 1)
			{
				printf("stop\n");

			}
			++count;*/
			return dataArray[index][frame * 2 + channelValue];
		}
		else if (channels == 1) {
			return dataArray[index][frame];
		}
		else {
			throw std::exception();
		}
	}