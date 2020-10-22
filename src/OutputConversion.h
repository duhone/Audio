#pragma once

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

		void ConvertSampleRate(int32_t a_deviceSampleRate);

	  private:
		SRC_STATE* srcState = nullptr;
	};
}    // namespace CR::Audio
