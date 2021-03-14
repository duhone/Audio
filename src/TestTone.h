#pragma once

#include "Sample.h"

#include <core/Span.h>

#include <array>
#include <cstdint>

namespace CR::Audio {
	class TestTone {
	  public:
		TestTone(float a_frequency) : m_frequency(a_frequency) {}

		[[nodiscard]] Core::Span<Sample> GetSamples(uint32_t& a_currentSample, uint32_t a_numSamples);

	  private:
		float m_frequency = 0.0f;
		std::vector<Sample> m_buffer;
	};
}    // namespace CR::Audio
