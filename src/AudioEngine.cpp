#include "Audio/AudioEngine.h"

#include "AudioDevice.h"
#include "Sample.h"
#include "TestTone.h"

#include <core/Span.h>

#include <memory>

using namespace CR;
using namespace CR::Audio;

namespace {
	struct Engine {
		bool Mix(Core::Span<float>& a_buffer, bool a_closing);

		std::unique_ptr<AudioDevice> m_device;
		std::vector<Sample> m_buffer;
		TestTone m_testTone{1000.0f};
	};
	Engine& GetEngine() {
		static Engine s_engine;
		return s_engine;
	}
}    // namespace

bool Engine::Mix(Core::Span<float>& a_buffer, bool a_closing) {
	m_buffer.resize(std::min(a_buffer.size(), m_buffer.size()));
	m_testTone.Mix({m_buffer.data(), m_buffer.size()});
	if(a_closing) {
		return true;
	} else {
		return false;
	}
}

void Audio::EngineStart() {
	Engine& engine = GetEngine();

	engine.m_device = std::make_unique<AudioDevice>(
	    [&engine](Core::Span<float>& a_buffer, bool a_closing) { return engine.Mix(a_buffer, a_closing); });
}

void Audio::EngineStop() {
	Engine& engine = GetEngine();

	engine.m_device.reset();
}
