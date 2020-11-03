#include "OutputConversion.h"

#include "Constants.h"

#include <core/Log.h>

#include <3rdParty/samplerate.h>

using namespace CR;
using namespace CR::Audio;

OutputConversion::OutputConversion() {
	int error  = 0;
	m_srcState = src_new(SRC_SINC_FASTEST, c_mixChannels, &error);
	Core::Log::Require(m_srcState, "Failed to create lib sample rate state: error: {}", src_strerror(error));
}

OutputConversion::~OutputConversion() {
	src_delete(m_srcState);
}

void OutputConversion::ConvertSampleRate(int32_t a_deviceSampleRate, Core::Span<const Sample> a_input,
                                         Core::Span<Sample> a_output) {
	SRC_DATA srcData;
	srcData.end_of_input = 0;
	srcData.src_ratio    = static_cast<double>(a_deviceSampleRate) / c_mixSampleRate;
	srcData.data_in      = (float*)a_input.data();
	srcData.input_frames = (long)a_input.size();

	srcData.data_out      = (float*)a_output.data();
	srcData.output_frames = (long)a_output.size();

	int error = src_process(m_srcState, &srcData);
	Core::Log::Require(error == 0, "failed to resample audio: {}", src_strerror(error));
	Core::Log::Require(srcData.input_frames_used == a_input.size(), "resampling must consume all input currently");

	int silence = (int)a_output.size() - srcData.output_frames_gen;
	if(silence > 0) {
		Core::Log::Require(m_firstFrame, "only the first frame should have an incomplete resample result");
		memmove(a_output.data() + silence, a_output.data(), srcData.output_frames_gen * sizeof(Sample));
		memset(a_output.data(), 0, silence * sizeof(Sample));
	}
	m_firstFrame = false;
}

void OutputConversion::ConvertChannelCount(const Core::Span<Sample> a_input, Core::Span<float> a_output,
                                           const std::vector<ChannelWeights>& a_weights) const {
	Core::Log::Assert(a_weights.size() * a_input.size() == a_output.size(), "Invalid input to ConvertChannelCount");

	int32_t inputStep = static_cast<int32_t>(a_weights.size());
	for(int32_t sample = 0; sample < a_input.size(); ++sample) {
		for(int32_t weight = 0; weight < inputStep; ++weight) {
			a_output[sample * inputStep + weight] =
			    a_weights[weight].Left * a_input[sample].Left + a_weights[weight].Right * a_input[sample].Right;
		}
	}
}
