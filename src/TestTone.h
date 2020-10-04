#pragma once

#include "Sample.h"

#include <core/Span.h>

#include <cstdint>

namespace CR::Audio {
	class TestTone {
	  public:
		TestTone(float a_frequency) : m_frequency(a_frequency) {}

		void Mix(Core::Span<Sample> a_buffer);

	  private:
		float m_frequency        = 0.0f;
		uint32_t m_currentSample = 0;
	};
}    // namespace CR::Audio
