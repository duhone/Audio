#include <3rdParty/doctest.h>

#include "Audio/AudioEngine.h"

using namespace CR::Audio;

TEST_CASE("engine creation/destruction") {
	EngineStart();
	EngineStop();
}
