#pragma once

#include "win32AudioApiInterfaceDLLheader.h"
#include "win32AudioCallbackUserDataStruct.h"

#include <cstdint>
#include <vector>
#include <array>
#include "win32AudioApiPlayDoneNotificationTypeEnumerance.h"
#include "eventManagementInterface.h"
#include "noCopyAndMoveTemplate.h"

namespace Win32AudioSystem {
	constexpr std::uint8_t audioDeviceBufferQuantity = 4;
	class win32AudioApiInterface_API   ApiInterface 
	: public NoCopyAndMove<ApiInterface>
	{
	private:
		const PlayDoneNotificationType playDoneNotificationMode = PlayDoneNotificationType::thread;
		MMRESULT returnValue = MMSYSERR_NOERROR;
		//4 - 8KB?
		std::uint32_t framesPerPeriod = 600;
		std::uint8_t deviceChannelQuantity = 2;
		std::uint32_t deviceSamplingRate = 44100;
		HWAVEOUT waveformAudioOutputDevice = nullptr;
		bool waveOutOpenFlag = false;
		std::uint8_t audioDeviceBufferCurrentIndex = 0;
		std::uint8_t audioDeviceBufferEndMaxCount = 0;
		std::uint8_t audioDeviceBufferEndCount = 0;
		WAVEFORMATEX deviceDataSetting = { 0 };
		std::array<std::vector<std::int16_t>, audioDeviceBufferQuantity> audioDeviceBufferArray;
		std::array<WAVEHDR, audioDeviceBufferQuantity> waveformAudioHeaderArray;
		std::array<MyUtility::EventManagementInterface, audioDeviceBufferQuantity>   audioBufferPlayDoneEventArray;

		std::uint16_t waveOutOpenWrap(DWORD_PTR dwCallback, DWORD_PTR dwInstance,
			DWORD  fdwOpen);
		std::uint16_t waveOutPrepareHeaderWrap(HWAVEOUT waveformAudioOutputDevice, WAVEHDR & waveformAudioHeader);
		std::uint16_t waveOutWriteWrap(HWAVEOUT waveformAudioOutputDevice, WAVEHDR & waveformAudioHeader);

		std::uint16_t	waveOutPauseWrap();
		std::uint16_t	waveOutRestartWrap();
		std::uint16_t	waveOutResetWrap();
		std::uint16_t	waveOutUnprepareHeaderWrap(HWAVEOUT waveformAudioOutputDevice,
			WAVEHDR &waveformAudioHeader);

		std::uint16_t	waveOutCloseWrap();
		std::uint16_t  prepareAllAudioHeader();
		std::uint16_t prepareAudioHeader(std::uint32_t index,
			std::int16_t *primaryBufferDataPtr);
		std::uint16_t prepareAudioHeader(
			WAVEHDR& waveformAudioHeader, std::int16_t *audioBufferDataPtr);
		std::uint16_t
			writeDataToAudioDeviceBuffer(HWAVEOUT waveformAudioOutputDevice,
				WAVEHDR& waveformAudioHeader,
				std::int16_t *audioBufferDataPtr);

	public:
		ApiInterface() = default;
		PlayDoneNotificationType getPlayDoneNotificationMethod() const noexcept;
		std::uint16_t initialize();
		std::uint32_t getFramesPerPeriod() const noexcept;
		std::uint8_t getDeviceChannelQuantity() const noexcept;
		std::uint32_t getDeviceSamplingRate() const noexcept;
		std::uint8_t getAudioBufferQuantity() const noexcept;
	    MyUtility::EventManagementInterface&  getAudioBufferPlayDoneEventRef(std::uint32_t
			index) noexcept;

		std::uint16_t preparePlayByThreadNotification(DWORD thread);
		std::uint16_t preparePlayByCallbackNotification();
	    void updateAudioDeviceBufferCurrentIndex() noexcept;

		std::uint16_t writeDataToAudioDeviceBuffer(std::int16_t *audioBufferDataPtr);
		void setAudioDeviceBufferEndMaxCount(std::uint8_t count) noexcept;
		std::uint16_t audioMessageTask(MSG& messageInfo);
		std::uint16_t
			writeDataToAudioDeviceBufferWrap(WPARAM wParma,
				LPARAM lParam,
				std::int16_t *audioBufferDataPtr);
		std::uint16_t audioEventTask();
		bool checkPlayEndAfterPoolEnd();
		std::uint16_t taskAfterPlayEnd();
		std::uint16_t finalize();
		~ApiInterface();
	};
}