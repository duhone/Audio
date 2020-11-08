#include <MixerSystem.h>

using namespace CR;

[[nodiscard]] Audio::Mixer Audio::MixerSystem::CreateMixer() {
	return Mixer{};
}

// Handle class

Audio::Mixer::~Mixer() {}

Audio::Mixer& Audio::Mixer::operator=(Audio::Mixer&& a_other) noexcept {
	if(m_id >= 0) { this->~Mixer(); }

	m_id     = a_other.m_id;
	m_system = a_other.m_system;

	a_other.m_id = c_unused;

	return *this;
}
