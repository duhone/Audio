#include <AudioSets.h>

#include <core/Log.h>

#include <bitset>

using namespace CR::Audio;

namespace {
	struct AudioSetsData {
		AudioSetsData()  = default;
		~AudioSetsData() = default;

		AudioSetsData(const AudioSetsData&) = delete;
		AudioSetsData(AudioSetsData&&)      = delete;
		AudioSetsData& operator=(const AudioSetsData&) = delete;
		AudioSetsData& operator=(AudioSetsData&&) = delete;

		std::bitset<AudioSet::c_maxSets> m_setUsed;
		std::array<std::string, AudioSet::c_maxSets> m_setNames;
		std::array<uint32_t, AudioSet::c_maxSets> m_ids;
	};

	uint16_t GetSet(uint16_t a_key) { return a_key >> 12; }
	uint16_t GetStream(uint16_t a_key) { return a_key & 0x7fff; }

	std::unique_ptr<AudioSetsData> g_audioSets;
}    // namespace

AudioSetsSystem::AudioSetsSystem() {
	Core::Log::Require(!g_audioSets, "Only one AudioSetsSystem is allowed");
	g_audioSets = std::make_unique<AudioSetsData>();
}

AudioSetsSystem::~AudioSetsSystem() {
	Core::Log::Require((bool)g_audioSets, "AudioSetsSystem was destructed while in an invalid state");
	g_audioSets.reset();
}

AudioSet::AudioSet(const std::string_view& a_setName, uint32_t a_id, Core::Span<AudioCreateInfo> a_audioStreams) {
	Core::Log::Require(a_audioStreams.size() <= c_maxStreamsPerSet,
	                   "Audio Set {} has {} audio streams which is higher than the maximum of {}", a_setName,
	                   a_audioStreams.size(), c_maxStreamsPerSet);

	Core::Log::Require((bool)g_audioSets, "Audio Sets not initialized");

	uint16_t set = c_unused;
	for(uint16_t i = 0; i < c_maxSets; ++i) {
		if(g_audioSets->m_setUsed[i]) { set = i; }
	}
	Core::Log::Require(set != c_unused, "ran out of audio sets");

	g_audioSets->m_setUsed[set]  = true;
	g_audioSets->m_setNames[set] = a_setName;
	g_audioSets->m_ids[set]      = a_id;

	m_key = set;
}

AudioSet::~AudioSet() {
	Core::Log::Require((bool)g_audioSets, "Audio Sets not initialized");

	if(m_key == c_unused) { return; }

	g_audioSets->m_setUsed[m_key] = false;
	g_audioSets->m_setNames[m_key].clear();
	g_audioSets->m_setNames[m_key].shrink_to_fit();
}

AudioSet::AudioSet(AudioSet&& a_other) noexcept {
	*this = std::move(a_other);
}

AudioSet& AudioSet::operator=(AudioSet&& a_other) noexcept {
	m_key         = a_other.m_key;
	a_other.m_key = c_unused;
	return *this;
}
