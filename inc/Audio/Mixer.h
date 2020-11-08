#pragma once

#include <cinttypes>
#include <utility>

namespace CR::Audio {
	class MixerSystem;

	class Mixer {
	  public:
		// A default constructed Mixer isn't usable, must obtain a usable one by grabbing the master mixer, or creating
		// a mixer from another one(like the master)
		Mixer() = default;
		~Mixer();

		Mixer(const Mixer&) = delete;
		Mixer& operator=(const Mixer&) = delete;
		Mixer(Mixer&& a_other) noexcept { *this = std::move(a_other); }
		Mixer& operator=(Mixer&& a_other) noexcept;

	  private:
		constexpr static int32_t c_unused{-1};

		int32_t m_id{c_unused};
		MixerSystem* m_system{nullptr};
	};
}    // namespace CR::Audio
