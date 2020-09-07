#include "Audio/AudioEngine.h"

#include "AudioDevice.h"

#include <memory>

using namespace CR;

namespace {
	std::unique_ptr<Audio::AudioDevice>& GetDevice() {
		static std::unique_ptr<Audio::AudioDevice> s_device;
		return s_device;
	}
}    // namespace

void Audio::EngineStart() {
	GetDevice() = std::make_unique<Audio::AudioDevice>();
}

void Audio::EngineStop() {
	GetDevice().release();
}
