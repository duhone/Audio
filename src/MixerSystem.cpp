#include <MixerSystem.h>

using namespace CR;

[[nodiscard]] Audio::MixerHandle Audio::MixerSystem::CreateMixer() {
	return MixerHandle{};
}

// Handle class

Audio::MixerHandle::~MixerHandle() {}

Audio::MixerHandle& Audio::MixerHandle::operator=(Audio::MixerHandle&& a_other) noexcept {
	if(m_id >= 0) { this->~MixerHandle(); }

	m_id     = a_other.m_id;
	m_system = a_other.m_system;

	a_other.m_id = c_unused;

	return *this;
}
