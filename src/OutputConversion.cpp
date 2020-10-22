#include "OutputConversion.h"

#include "Constants.h"

#include <core/Log.h>

#include <3rdParty/samplerate.h>

using namespace CR;
using namespace CR::Audio;

OutputConversion::OutputConversion() {
	int error = 0;
	srcState  = src_new(SRC_SINC_FASTEST, c_mixChannels, &error);
	Core::Log::Require(srcState, "Failed to create lib sample rate state: error: {}", src_strerror(error));
}

OutputConversion::~OutputConversion() {
	src_delete(srcState);
}

void OutputConversion::ConvertSampleRate(int32_t a_deviceSampleRate) {
	SRC_DATA srcData;
	srcData.src_ratio = static_cast<double>(a_deviceSampleRate) / c_mixSampleRate;
}
