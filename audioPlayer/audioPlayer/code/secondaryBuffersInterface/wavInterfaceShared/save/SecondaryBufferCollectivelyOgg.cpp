#include "SecondaryBufferCollectivelyOgg.h"

	void SecondaryBufferCollectivelyOgg::initialize() {
		
		
		std::thread t([&] {	
		FILE  *fp;

		errno_t error = tfopen_s(&fp, filePath.c_str(), TEXT("rb"));

		if (error != 0) {
				tprintf(TEXT("ファイルを開けませんでした: %s\n"), filePath.c_str());
			throw std::exception();
		}
		oggVorbisInstance.reset(new OggVorbisCollectively(fp, oggSamplesArray));
		oggVorbisInstance->oggTask();

		fclose(fp);
		ready = true;
		//SecondaryBuffer::start();

		});

		t.detach();
	}

	HANDLE SecondaryBufferCollectivelyOgg::getEvent(unsigned char i) { return NULL; };

	SecondaryBufferCollectivelyOgg::SecondaryBufferCollectivelyOgg(ReadingMethod method, Endian endianFlag, std::TSTRING filePath,
		PaStream * stream, bool loop, std::mutex* startStreamMutexPt) :SecondaryBuffer(method, endianFlag, filePath, stream, loop, startStreamMutexPt), currentStream(0),currentIndex(0) {

		this->filePath = filePath;
		initialize();
	}


	int16_t  SecondaryBufferCollectivelyOgg::getBufferData(DataChannel channelSide, uint32_t frame) {
		//Prevent unused variable warnings
		(void)frame;
		if (currentIndex == oggSamplesArray[currentStream].second.size()) {
			if (currentStream == (oggSamplesArray.size() - 1))
			{
				//system("pause");
				//printf("-------------file end\n");
				currentStream = 0;
				if (!loop) {
					printf(" to no play");
					play = false;
				}
			}
			else {
				++currentStream;


			}
			currentIndex = 0;
		}
		if (!play) {

			//printf("no play");
			return 0;


		}

		//( "chanels: %u" , channels);

		if (oggSamplesArray[currentStream].first == 2) {
			return oggSamplesArray[currentStream].second[currentIndex++];
		}
		else if (oggSamplesArray[currentStream].first == 1) {
			//printf("%d\n",sampleIndex );
			if (channelSide == DataChannel::left) {
				return oggSamplesArray[currentStream].second[currentIndex];
			}
			else if (channelSide == DataChannel::right) {
				return oggSamplesArray[currentStream].second[currentIndex++];
			}
		}
		else {
			printf("programing error\n");
			return 0;
			exit(1);
		}
		return 0;
	}