#pragma once

#include <Audio/Mixer.h>

namespace CR::Audio {
	class MixerSystem {
	  public:
		MixerSystem()  = default;
		~MixerSystem() = default;

		MixerSystem(const MixerSystem&)    = delete;
		MixerSystem(MixerSystem&& a_other) = delete;

		MixerSystem& operator=(const MixerSystem&) = delete;
		MixerSystem& operator=(MixerSystem&&) = delete;

		[[nodiscard]] Mixer CreateMixer();

	  private:
	};
}    // namespace CR::Audio
