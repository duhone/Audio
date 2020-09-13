#include <3rdParty/doctest.h>

#include "Audio/AudioEngine.h"

#include <chrono>
#include <thread>

using namespace CR::Audio;
using namespace std::chrono_literals;

TEST_CASE("engine creation/destruction") {
	EngineStart();

	std::this_thread::sleep_for(1s);

	EngineStop();
}
