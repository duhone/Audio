#pragma once

#include <atlbase.h>
#include <audioclient.h>

#include <cstdint>
#include <memory>

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
		std::unique_ptr<class AudioDeviceImpl> m_pimpl;
	};
}    // namespace CR::Audio
