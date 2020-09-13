#pragma once

#include <memory>

namespace CR::Audio {
	class AudioDevice {
	  public:
		AudioDevice();
		~AudioDevice();

		AudioDevice(const AudioDevice&) = delete;
		AudioDevice(AudioDevice&&)      = delete;
		AudioDevice& operator=(const AudioDevice&) = delete;
		AudioDevice& operator=(AudioDevice&&) = delete;

	  private:
		std::unique_ptr<class AudioDeviceImpl> m_pimpl;
	};
}    // namespace CR::Audio
