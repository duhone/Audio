#pragma once

#include <ChannelWeights.h>

#include <core/Span.h>

#include <3rdParty/function2.h>

#include <memory>
#include <vector>

namespace CR::Audio {
	class AudioDevice {
	  public:
		// a_calling is true if starting the shutdown process, clients should start a final "fade out"
		// callback should return true if it has filled the buffer, if false is returned the device will fill with
		// silence, this should be the case during closing after finishing any final fade out, can also use if just
		// don't have anything to mix though, as the device may be able to signal silence to the hardware in a more
		// efficient method than just filling the buffer with 0'z.
		// Do not access the device during this callback.
		// a_weights: This returns how much of each of the stereo channels should be mixed for that output channel from
		// the engine standard stereo source data. will have exactly a_numChannels entries.
		using DeviceCallback_t =
		    fu2::unique_function<bool(Core::Span<float> a_buffer, int32_t a_numChannels, int32_t a_sampleRate,
		                              const std::vector<ChannelWeights> a_weights, bool a_closing)>;

		AudioDevice(DeviceCallback_t a_callback);
		~AudioDevice();

		AudioDevice(const AudioDevice&) = delete;
		AudioDevice(AudioDevice&&)      = delete;
		AudioDevice& operator=(const AudioDevice&) = delete;
		AudioDevice& operator=(AudioDevice&&) = delete;

	  private:
		std::unique_ptr<class AudioDeviceImpl> m_pimpl;
	};
}    // namespace CR::Audio
