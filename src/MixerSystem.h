#pragma once

#include <Audio/MixerHandle.h>

#include <Mixer.h>

#include <bitset>

namespace CR::Audio {
	class MixerSystem {
	  public:
		MixerSystem()  = default;
		~MixerSystem() = default;

		MixerSystem(const MixerSystem&)    = delete;
		MixerSystem(MixerSystem&& a_other) = delete;

		MixerSystem& operator=(const MixerSystem&) = delete;
		MixerSystem& operator=(MixerSystem&&) = delete;

		[[nodiscard]] MixerHandle CreateMixer();

	  private:
		constexpr static int32_t c_maxMixers = 16;

		std::bitset<c_maxMixers> m_used;
		Mixer m_mixers[c_maxMixers];
	};
}    // namespace CR::Audio
