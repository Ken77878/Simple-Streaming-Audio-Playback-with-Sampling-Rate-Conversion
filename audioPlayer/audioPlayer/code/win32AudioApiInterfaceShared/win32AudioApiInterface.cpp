#include "win32AudioApiInterface.h"
#include "characterCodeMacro.h"
#include <string>
#include "windows/timeMeasurement.h"

namespace Win32AudioSystem {
	std::uint16_t ApiInterface::initialize() {
		waveformAudioHeaderArray.fill({ 0 });

		deviceDataSetting.wFormatTag = WAVE_FORMAT_PCM;
		deviceDataSetting.nChannels = deviceChannelQuantity;
		deviceDataSetting.nSamplesPerSec = deviceSamplingRate;
		deviceDataSetting.wBitsPerSample = 16;
		deviceDataSetting.nAvgBytesPerSec =
			(deviceDataSetting.nSamplesPerSec * deviceDataSetting.nChannels *
				deviceDataSetting.wBitsPerSample) >>
			3;
		deviceDataSetting.nBlockAlign =
			(deviceDataSetting.nChannels * deviceDataSetting.wBitsPerSample) >> 3;
		deviceDataSetting.cbSize = 0;

		for (std::uint32_t u = 0; u < audioDeviceBufferQuantity; ++u)
		{
			audioDeviceBufferArray[u].resize(framesPerPeriod * deviceChannelQuantity);

		}
		if (playDoneNotificationMode == PlayDoneNotificationType::callback) {
			//	audioBufferPlayDoneEventArray.resize(audioDeviceBufferQuantity);
			for (std::uint32_t u = 0; u < audioDeviceBufferQuantity; ++u)
				if (audioBufferPlayDoneEventArray[u].initialize
				(nullptr, false,
					false, nullptr)) {
					return 1;
				}
		}
		return 0;
	}

	void ApiInterface::setAudioDeviceBufferEndMaxCount(std::uint8_t count) noexcept {
		if (count > audioDeviceBufferQuantity)
			audioDeviceBufferEndMaxCount = audioDeviceBufferQuantity;
		else audioDeviceBufferEndMaxCount = count;

	}


	PlayDoneNotificationType ApiInterface::getPlayDoneNotificationMethod() const
		noexcept {
		return playDoneNotificationMode;
	}
	std::uint32_t ApiInterface::getFramesPerPeriod() const noexcept {
		return framesPerPeriod;
	}
	std::uint8_t ApiInterface::getDeviceChannelQuantity() const noexcept {
		return deviceChannelQuantity;
	}
	std::uint32_t ApiInterface::getDeviceSamplingRate() const noexcept {
		return deviceSamplingRate;
	}
	std::uint8_t ApiInterface::getAudioBufferQuantity() const noexcept {
		return audioDeviceBufferQuantity;
	}

	std::uint16_t ApiInterface::waveOutOpenWrap(DWORD_PTR dwCallback,
		DWORD_PTR dwInstance,
		DWORD fdwOpen) {
		if (waveOutOpenFlag)
			return 0;
		if ((returnValue = waveOutOpen(&waveformAudioOutputDevice, WAVE_MAPPER,
			&deviceDataSetting, dwCallback, dwInstance,
			fdwOpen)) != MMSYSERR_NOERROR) {
			std::TSTRING errorMessage = TEXT("waveOutOpen() failed, :");
			switch (returnValue) {
			case MMSYSERR_ALLOCATED:
				errorMessage += TEXT("MMSYSERR_ALLOCATED");
				break;

			case MMSYSERR_BADDEVICEID:
				errorMessage += TEXT("MMSYSERR_BADDEVICEID");
				break;
			case MMSYSERR_NODRIVER:
				errorMessage += TEXT("MMSYSERR_NODRIVER");
				break;
			case MMSYSERR_NOMEM:
				errorMessage += TEXT("MMSYSERR_NOMEM");
				break;
			case WAVERR_BADFORMAT:
				errorMessage += TEXT("WAVERR_BADFORMAT");
				break;
			case WAVERR_SYNC:
				errorMessage += TEXT("WAVERR_SYNC");
				break;
			default:
				errorMessage += TEXT("unknown error");
			}
			tfprintf(stderr, TEXT("%s"), errorMessage.c_str());
			return 1;
		}
		waveOutOpenFlag = true;
		return 0;
	}

	std::uint16_t
		ApiInterface::waveOutUnprepareHeaderWrap(HWAVEOUT waveformAudioOutputDevice,
			WAVEHDR &waveformAudioHeader) {

		if ((returnValue = waveOutUnprepareHeader(waveformAudioOutputDevice,
			&waveformAudioHeader, sizeof(WAVEHDR)

		)) != MMSYSERR_NOERROR)

		{
			std::TSTRING errorMessage = TEXT("waveOutUnprepareHeader() failed, :");
			switch (returnValue) {
			case MMSYSERR_INVALHANDLE:
				errorMessage += TEXT("MMSYSERR_INVALHANDLE");
				break;
			case MMSYSERR_NODRIVER:
				errorMessage += TEXT("MMSYSERR_NODRIVER");
				break;
			case MMSYSERR_NOMEM:
				errorMessage += TEXT("MMSYSERR_NOMEM");
				break;
			case WAVERR_STILLPLAYING:
				errorMessage += TEXT("WAVERR_STILLPLAYING");
				break;
			case MMSYSERR_INVALPARAM:
				errorMessage += TEXT("WAVERR_INVALPARAM");
				break;
			default:
				errorMessage += TEXT("unknown error:") + std::to_tstring(returnValue);
			}
			errorMessage += TEXT("\n");
			tfprintf(stderr, TEXT("%s"), errorMessage.c_str());
			return 1;
		}
		return 0;
	}

	std::uint16_t
		ApiInterface::waveOutPrepareHeaderWrap(HWAVEOUT waveformAudioOutputDevice,
			WAVEHDR &waveformAudioHeader) {

		if ((returnValue = waveOutPrepareHeader(waveformAudioOutputDevice,
			&waveformAudioHeader, sizeof(WAVEHDR)

		)) != MMSYSERR_NOERROR)

		{
			std::TSTRING errorMessage = TEXT("waveOutPrepareHeader() failed, :");
			switch (returnValue) {
			case MMSYSERR_INVALHANDLE:
				errorMessage += TEXT("MMSYSERR_INVALHANDLE");
				break;
			case MMSYSERR_NODRIVER:
				errorMessage += TEXT("MMSYSERR_NODRIVER");
				break;
			case MMSYSERR_NOMEM:
				errorMessage += TEXT("MMSYSERR_NOMEM");
				break;
			default:
				errorMessage += TEXT("unknown error");
			}
			errorMessage += TEXT("\n");
			tfprintf(stderr, TEXT("%s"), errorMessage.c_str());
			return 1;
		}
		return 0;
	}

	std::uint16_t ApiInterface::waveOutWriteWrap(HWAVEOUT waveformAudioOutputDevice,
		WAVEHDR &waveformAudioHeader) {
		if ((returnValue = waveOutWrite(waveformAudioOutputDevice,
			&waveformAudioHeader, sizeof(WAVEHDR))) !=
			MMSYSERR_NOERROR)

		{
			std::TSTRING errorMessage = TEXT("waveOutWrite() failed, :");
			switch (returnValue) {
			case MMSYSERR_INVALHANDLE:
				errorMessage += TEXT("MMSYSERR_INVALHANDLE");
				break;
			case MMSYSERR_NODRIVER:
				errorMessage += TEXT("MMSYSERR_NODRIVER");
				break;
			case MMSYSERR_NOMEM:
				errorMessage += TEXT("MMSYSERR_NOMEM");
				break;
			case WAVERR_UNPREPARED:
				errorMessage += TEXT("WAVERR_UNPREPARED");
				break;
			default:
				errorMessage += TEXT("unknown error");
			}
			errorMessage += TEXT("\n");
			tfprintf(stderr, TEXT("%s"), errorMessage.c_str());
			return 1;
		}
		return 0;
	}

	std::uint16_t ApiInterface::waveOutPauseWrap() {
		if ((returnValue = waveOutPause(waveformAudioOutputDevice)) !=
			MMSYSERR_NOERROR)

		{
			std::TSTRING errorMessage = TEXT("waveOutPause() failed, :");
			switch (returnValue) {
			case MMSYSERR_INVALHANDLE:
				errorMessage += TEXT("MMSYSERR_INVALHANDLE");
				break;
			case MMSYSERR_NODRIVER:
				errorMessage += TEXT("MMSYSERR_NODRIVER");
				break;
			case MMSYSERR_NOMEM:
				errorMessage += TEXT("MMSYSERR_NOMEM");
				break;
			case MMSYSERR_NOTSUPPORTED:
				errorMessage += TEXT("MMSYSERR_NOTSUPPORTED");
				break;
			default:
				errorMessage += TEXT("unknown error");
			}
			tprintf(TEXT("%s"), errorMessage.c_str());
			return 1;
		}
		return 0;
	}
	std::uint16_t ApiInterface::waveOutRestartWrap() {
		if ((returnValue = waveOutRestart(waveformAudioOutputDevice)) !=
			MMSYSERR_NOERROR)

		{
			std::TSTRING errorMessage = TEXT("waveOutRestart() failed, :");
			switch (returnValue) {
			case MMSYSERR_INVALHANDLE:
				errorMessage += TEXT("MMSYSERR_INVALHANDLE");
				break;
			case MMSYSERR_NODRIVER:
				errorMessage += TEXT("MMSYSERR_NODRIVER");
				break;
			case MMSYSERR_NOMEM:
				errorMessage += TEXT("MMSYSERR_NOMEM");
				break;
			case MMSYSERR_NOTSUPPORTED:
				errorMessage += TEXT("MMSYSERR_NOTSUPPORTED");
				break;
			default:
				errorMessage += TEXT("unknown error");
			}
			errorMessage += TEXT("\n");
			tfprintf(stderr, TEXT("%s"), errorMessage.c_str());
			return 1;
		}
		return 0;
	}
	std::uint16_t ApiInterface::waveOutResetWrap() {
		if ((returnValue = waveOutReset(waveformAudioOutputDevice)) !=
			MMSYSERR_NOERROR) {
			std::TSTRING errorMessage = TEXT("waveOutReset() failed, :");
			switch (returnValue) {
			case MMSYSERR_INVALHANDLE:
				errorMessage += TEXT("MMSYSERR_INVALHANDLE");
				break;
			case MMSYSERR_NODRIVER:
				errorMessage += TEXT("MMSYSERR_NODRIVER");
				break;
			case MMSYSERR_NOMEM:
				errorMessage += TEXT("MMSYSERR_NOMEM");
				break;
			case MMSYSERR_NOTSUPPORTED:
				errorMessage += TEXT("MMSYSERR_NOTSUPPORTED");
				break;
			default:
				errorMessage += TEXT("unknown error");
			}
			errorMessage += TEXT("\n");
			tfprintf(stderr, TEXT("%s"), errorMessage.c_str());
			return 1;
		}
		return 0;
	}

	std::uint16_t ApiInterface::waveOutCloseWrap() {
		if (!waveformAudioOutputDevice)
			return 0;
		if ((returnValue = waveOutClose(waveformAudioOutputDevice)) !=
			MMSYSERR_NOERROR) {
			std::TSTRING errorMessage = TEXT("waveOutClose() failed, :");
			switch (returnValue) {
			case MMSYSERR_INVALHANDLE:
				errorMessage += TEXT("MMSYSERR_INVALHANDLE");
				break;
			case MMSYSERR_NODRIVER:
				errorMessage += TEXT("MMSYSERR_NODRIVER");
				break;
			case MMSYSERR_NOMEM:
				errorMessage += TEXT("MMSYSERR_NOMEM");
				break;
			case WAVERR_STILLPLAYING:
				errorMessage += TEXT("WAVERR_STILLPLAYING");
				break;
			default:
				errorMessage += TEXT("unknown error");
			}
			errorMessage += TEXT("\n");
			tprintf(TEXT("%s"), errorMessage.c_str());
			return 1;
		}
		waveformAudioOutputDevice = nullptr;
		waveOutOpenFlag = false;
		return 0;
	}

	MyUtility::EventManagementInterface&   ApiInterface::getAudioBufferPlayDoneEventRef(std::uint32_t
		index) noexcept {
		return audioBufferPlayDoneEventArray[index];
	}

	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance,
		DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
		switch (uMsg) {
		case WOM_OPEN:
			break;
		case WOM_DONE: {
			if (
				reinterpret_cast<ApiInterface *>(
					dwInstance)->getAudioBufferPlayDoneEventRef(reinterpret_cast<WAVEHDR *>(dwParam1)->dwUser)
				.setEventOn()) {
				_exit(333);
			}
		} break;
		case WOM_CLOSE:
			break;
		}
	}

	std::uint16_t  ApiInterface::prepareAllAudioHeader() {
		for (std::uint32_t u = 0; u < audioDeviceBufferQuantity; ++u)
			if (prepareAudioHeader(u, &audioDeviceBufferArray[u][0]))
				return 1;
		return 0;
	}
	std::uint16_t ApiInterface::preparePlayByThreadNotification(DWORD threadId) {
		if (waveOutOpenWrap(threadId, 0, CALLBACK_THREAD))
			return 1;
		if (prepareAllAudioHeader())
			return 1;
		return 0;
	}

	std::uint16_t ApiInterface::preparePlayByCallbackNotification() {
		if (waveOutOpenWrap(reinterpret_cast<DWORD_PTR>(waveOutProc),
			reinterpret_cast<DWORD_PTR>(this),
			CALLBACK_FUNCTION))
			return 1;
		if (prepareAllAudioHeader())
			return 1;
		return 0;
	}


	std::uint16_t ApiInterface::prepareAudioHeader(
		std::uint32_t index, std::int16_t *audioBufferDataPtr)
	{

		waveformAudioHeaderArray[index].lpData = reinterpret_cast<LPSTR>(audioBufferDataPtr);
		waveformAudioHeaderArray[index].dwBufferLength =
			framesPerPeriod * deviceChannelQuantity * sizeof(std::int16_t);
		waveformAudioHeaderArray[index].dwFlags = 0;
		if (playDoneNotificationMode == PlayDoneNotificationType::callback) {
			waveformAudioHeaderArray[index].dwUser = index;
		}
		if (waveOutPrepareHeaderWrap(waveformAudioOutputDevice, waveformAudioHeaderArray[index]))
			return 1;

		return 0;
	}

	std::uint16_t ApiInterface::prepareAudioHeader(
		WAVEHDR& waveformAudioHeader, std::int16_t *audioBufferDataPtr)
	{
		waveformAudioHeader.lpData = reinterpret_cast<LPSTR>(audioBufferDataPtr);
		waveformAudioHeader.dwBufferLength =
			framesPerPeriod * deviceChannelQuantity * sizeof(std::int16_t);
		waveformAudioHeader.dwFlags = 0;
		if (waveOutPrepareHeaderWrap(waveformAudioOutputDevice, waveformAudioHeader))
			return 1;

		return 0;
	}


	std::uint16_t
		ApiInterface::writeDataToAudioDeviceBuffer(std::int16_t *audioBufferDataPtr) {
		
		std::memcpy(&audioDeviceBufferArray[audioDeviceBufferCurrentIndex][0],
			audioBufferDataPtr,
			sizeof(std::int16_t) * framesPerPeriod * deviceChannelQuantity
			);
		if (waveOutWriteWrap(waveformAudioOutputDevice, waveformAudioHeaderArray[audioDeviceBufferCurrentIndex]))
			return 1;
		return 0;
	}

	void ApiInterface::updateAudioDeviceBufferCurrentIndex() noexcept{
	if (++audioDeviceBufferCurrentIndex == audioDeviceBufferQuantity)
			audioDeviceBufferCurrentIndex = 0;
	}

	std::uint16_t
		ApiInterface::writeDataToAudioDeviceBuffer(HWAVEOUT waveformAudioOutputDevice,
			WAVEHDR& waveformAudioHeader,
			std::int16_t *audioBufferDataPtr) {
		std::memcpy(waveformAudioHeader.lpData,
			audioBufferDataPtr,
			sizeof(std::int16_t) * framesPerPeriod * deviceChannelQuantity
			);
		if (waveOutWriteWrap(waveformAudioOutputDevice, waveformAudioHeader))
			return 1;
		return 0;
	}

	std::uint16_t
		ApiInterface::writeDataToAudioDeviceBufferWrap(WPARAM wParam,
			LPARAM lParam,
			std::int16_t *audioBufferDataPtr) {
		if (writeDataToAudioDeviceBuffer(reinterpret_cast<HWAVEOUT>(wParam),
			*reinterpret_cast<WAVEHDR *>(lParam),
			audioBufferDataPtr
		))
			return 1;
		return 0;
	}

	bool ApiInterface::checkPlayEndAfterPoolEnd() {
		++audioDeviceBufferEndCount;
		if (audioDeviceBufferEndCount == (audioDeviceBufferEndMaxCount - 1)) {
			audioDeviceBufferEndCount = 0;
			return true;
		}
		else
			return false;
	}

	std::uint16_t ApiInterface::audioMessageTask(
		MSG &messageInfo) {
		for (;;) {
			if ((returnValue = ::GetMessage(&messageInfo, reinterpret_cast<HWND>(-1), 0, 0)) > 0)
			{
				switch (messageInfo.message) {
				case MM_WOM_OPEN:
					continue;
					break;
				case MM_WOM_DONE:
					return 0;
					break;
				case MM_WOM_CLOSE:
					continue;
					break;
				default:
					continue;
					break;
				}
			}
			else {
				if (!returnValue) {
					std::fprintf(stderr, "WM_QUIT came unexpectedly.");
				}
				else {
					std::fprintf(stderr, "GetMessage system error\n");
					//systemError
				}
				return 1;
			}
		}
		return 0;
	}

	std::uint16_t ApiInterface::audioEventTask() {
		if (audioBufferPlayDoneEventArray[audioDeviceBufferCurrentIndex].waitEvent()) {
			return 1;
		}
		return 0;
	}

	std::uint16_t ApiInterface::taskAfterPlayEnd() {
		if (waveOutResetWrap())
			return 1;

		for (std::uint32_t u = 0; u < audioDeviceBufferQuantity; ++u) {
			if (waveOutUnprepareHeaderWrap(waveformAudioOutputDevice,
				waveformAudioHeaderArray[u]))
				return 1;
		}
		if (waveOutCloseWrap())
			return 1;
		return 0;
	}

	std::uint16_t ApiInterface::finalize() {
		if (playDoneNotificationMode == PlayDoneNotificationType::callback) {
			for (std::uint32_t u = 0; u < audioDeviceBufferQuantity; ++u)
				if (audioBufferPlayDoneEventArray[u].finalize
				()) {
					return 1;
				}
		}
		return 0;
	}

	ApiInterface::~ApiInterface() {
#ifdef _DEBUG
		if (waveformAudioOutputDevice)
			exit(567);
#endif
	}
} // namespace Win32AudioSystem
