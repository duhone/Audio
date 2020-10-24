#pragma once

#include <Sample.h>

#include <core/Span.h>

using SRC_STATE = struct SRC_STATE_tag;

namespace CR::Audio {
	// Handle conversion for engine standard audio format, to devices format
	class OutputConversion {
	  public:
		OutputConversion();
		~OutputConversion();

		OutputConversion(const OutputConversion&) = delete;
		OutputConversion(OutputConversion&&)      = delete;

		OutputConversion& operator=(const OutputConversion&) = delete;
		OutputConversion& operator=(OutputConversion&&) = delete;

		void ConvertSampleRate(int32_t a_deviceSampleRate, Core::Span<const Sample> a_input,
		                       Core::Span<Sample> a_output);

	  private:
		SRC_STATE* m_srcState = nullptr;
		bool m_firstFrame     = true;
	};
}    // namespace CR::Audio
