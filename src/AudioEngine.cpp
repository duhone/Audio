#include "Audio/AudioEngine.h"

#include "AudioDevice.h"
#include "Constants.h"
#include "OutputConversion.h"
#include "Sample.h"
#include "TestTone.h"

#include <core/Span.h>

#include <memory>

using namespace CR;
using namespace CR::Audio;

namespace {
	struct Engine {
		bool Mix(Core::Span<float>& a_buffer, int32_t a_numChannels, int32_t a_sampleRate, bool a_closing);

		std::unique_ptr<AudioDevice> m_device;
		std::vector<Sample> m_mixBuffer;
		// Holds audio after main buffer conversion to device sample rate, if needed
		std::vector<Sample> m_deviceSampleBuffer;
		TestTone m_testTone{1000.0f};

		OutputConversion m_outputConversion;
	};
	Engine& GetEngine() {
		static Engine s_engine;
		return s_engine;
	}
}    // namespace

bool Engine::Mix(Core::Span<float>& a_buffer, int32_t a_numChannels, int32_t a_sampleRate, bool a_closing) {
	int32_t mixBufferSize = static_cast<int32_t>((a_buffer.size() * c_mixSampleRate) / (a_sampleRate * a_numChannels));
	m_mixBuffer.resize(mixBufferSize);
	m_testTone.Mix({m_mixBuffer.data(), m_mixBuffer.size()});

	Core::Span<Sample> resampleBuffer;
	if(a_sampleRate == c_mixSampleRate) {
		resampleBuffer = {m_mixBuffer.data(), m_mixBuffer.size()};
	} else {
		Core::Log::Error("not implemented");
	}

	Core::Span<float> deviceChannelBuffer;
	if(a_numChannels == c_mixChannels) {
		deviceChannelBuffer = {(float*)resampleBuffer.data(), resampleBuffer.size() * c_mixChannels};
	} else {
		Core::Log::Error("not implemented");
	}

	Core::Log::Require(deviceChannelBuffer.size() == a_buffer.size(),
	                   "Logic error, final buffer did not match device requirements");

	memcpy(a_buffer.data(), deviceChannelBuffer.data(), a_buffer.size() * sizeof(float));

	if(a_closing) {
		return true;
	} else {
		return false;
	}
}

void Audio::EngineStart() {
	Engine& engine = GetEngine();

	engine.m_device = std::make_unique<AudioDevice>(
	    [&engine](Core::Span<float>& a_buffer, int32_t a_numChannels, int32_t a_sampleRate, bool a_closing) {
		    return engine.Mix(a_buffer, a_numChannels, a_sampleRate, a_closing);
	    });
}

void Audio::EngineStop() {
	Engine& engine = GetEngine();

	engine.m_device.reset();
}
