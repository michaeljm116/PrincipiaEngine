#pragma once
#include <Artemis/Artemis.h>
#include <SDL_mixer.h>

namespace Principia {
	enum class AudioType {
		SoundEffect, Music
	};

	struct AudioComponent : public artemis::Component {
		bool play = false;
		Mix_Chunk* chunk;
		std::string fileName;//this should change some day

		AudioComponent(std::string s) : fileName(s) { chunk = nullptr; };
	};
}