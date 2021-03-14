#pragma once

#include <Audio/AudioSet.h>

#include <core/Span.h>

#include <string>

namespace CR::Audio {
	class AudioSetsSystem {
	  public:
		AudioSetsSystem();
		~AudioSetsSystem();
		AudioSetsSystem(const AudioSetsSystem&) = delete;
		AudioSetsSystem(AudioSetsSystem&&)      = delete;
		AudioSetsSystem& operator=(const AudioSetsSystem&) = delete;
		AudioSetsSystem& operator=(AudioSetsSystem&&) = delete;
	};
}    // namespace CR::Audio
