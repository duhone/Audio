#include "TestTone.h"

#include "Constants.h"

#include <cmath>

using namespace CR;
using namespace CR::Audio;

// remove once on C++20
static const float c_pi = 3.14159f;

void TestTone::Mix(Core::Span<Sample> a_buffer) {
	for(Sample& sample : a_buffer) {
		float toneSample = sin((2 * c_pi * m_frequency * m_currentSample) / (c_mixSampleRate));
		++m_currentSample;
		sample.Left  = toneSample;
		sample.Right = toneSample;
	}
}
