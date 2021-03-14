#include "TestTone.h"

#include "Constants.h"

#include <cmath>

using namespace CR;
using namespace CR::Audio;

// remove once on C++20
static const float c_pi = 3.14159f;

Core::Span<Sample> TestTone::GetSamples(uint32_t& a_currentSample, uint32_t a_numSamples) {
	m_buffer.resize(a_numSamples);

	for(Sample& sample : m_buffer) {
		float toneSample = sin((2 * c_pi * m_frequency * a_currentSample) / (c_mixSampleRate));
		++a_currentSample;
		sample.Left  = toneSample;
		sample.Right = toneSample;
	}

	return Core::Span<Sample>(std::data(m_buffer), std::size(m_buffer));
}
