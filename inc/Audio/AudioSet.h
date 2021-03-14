#pragma once

#include "core/Span.h"

#include <string>
#include <vector>

namespace CR::Audio {
	struct AudioCreateInfo {
		std::string StreamName;
		Core::Span<const std::byte> CRAudData;    // craud file
	};

	class AudioSet {
	  public:
		// Maximum that can be loaded at once
		inline static constexpr uint32_t c_maxSets          = 16;
		inline static constexpr uint32_t c_maxStreamsPerSet = 4095;

		AudioSet() = default;
		AudioSet(const std::string_view& a_setName, uint32_t a_id, Core::Span<AudioCreateInfo> a_audioStreams);
		~AudioSet();
		AudioSet(const AudioSet&) = delete;
		AudioSet(AudioSet&& a_other) noexcept;
		AudioSet& operator=(const AudioSet&) = delete;
		AudioSet& operator                   =(AudioSet&& a_other) noexcept;

	  private:
		inline static constexpr uint16_t c_unused{0xffff};

		uint16_t m_key{c_unused};
	};
}    // namespace CR::Audio
