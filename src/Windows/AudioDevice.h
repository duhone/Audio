#pragma once

#include <atlbase.h>
#include <audioclient.h>

#include <cstdint>

namespace CR::Audio {
	class AudioDevice {
	  public:
		AudioDevice() noexcept;
		~AudioDevice() noexcept;

		AudioDevice(const AudioDevice&) = delete;
		AudioDevice(AudioDevice&&)      = delete;
		AudioDevice& operator=(const AudioDevice&) = delete;
		AudioDevice& operator=(AudioDevice&&) = delete;

	  private:
		CComPtr<IAudioClient3> m_audioClient;
		CComPtr<IAudioRenderClient> m_audioRenderClient;
		uint32_t m_frameSamples = 0;
		uint32_t m_bufferFrames = 0;
		uint32_t m_frameSize    = 0;
		HANDLE m_audioEvent;
	};
}    // namespace CR::Audio
