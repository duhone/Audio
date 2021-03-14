#pragma once

#include <3rdParty/glm.h>

#include <memory>
#include <vector>

#include <Sample.h>

namespace CR::Audio {
	class Mixer {
	  public:
		Mixer()  = default;
		~Mixer() = default;

		Mixer(const Mixer&)    = delete;
		Mixer(Mixer&& a_other) = delete;

		Mixer& operator=(const Mixer&) = delete;
		Mixer& operator=(Mixer&&) = delete;

	  private:
		struct IAudioSource {
			virtual Sample* GetSamples(uint32_t a_numSamples) = 0;
		};

		template<typename T>
		class AudioSource : public IAudioSource {
		  public:
			AudioSource(T& a_source) : m_source(a_source) {}

		  private:
			Sample* GetSamples(uint32_t a_numSamples) override {
				Core::Span<Sample> samples = m_source.GetSamples(m_currentSample, a_numSamples);
				Core::Log::Require(samples.size() == a_numSamples,
				                   "GetSamples did not return exactly the requested number of samples");
				return std::data(samples);
			}

			T& m_source;
			uint32_t m_currentSample{0};
		};

		std::vector<std::shared_ptr<IAudioSource>> m_audioSources;
	};
}    // namespace CR::Audio
