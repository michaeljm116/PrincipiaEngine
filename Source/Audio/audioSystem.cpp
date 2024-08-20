#include "../pch.h"
#include "audioSystem.h"
#include <SDL_mixer.h>

namespace Principia {
	AudioSystem::AudioSystem()
	{
		addComponentType<AudioComponent>();
	}

	AudioSystem::~AudioSystem()
	{
		Mix_CloseAudio();
		Mix_Quit();
	}

	void AudioSystem::added(artemis::Entity & e)
	{
		AudioComponent* ac = audioMapper.get(e);
		ac->chunk = Mix_LoadWAV(ac->fileName.c_str());
		if (ac->chunk == nullptr) {
			std::cout << "ERROR: " + std::string(Mix_GetError());
		}
	}

	void AudioSystem::initialize()
	{
		audioMapper.init(*world);

		if (Mix_Init(MIX_INIT_MP3) == -1) {
			std::cout << "ERROR: " + std::string(Mix_GetError());
		}
		if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) {//CHUNCK SIZE MUST BE A VALUE OF 2
			std::cout << "ERRROR: " + std::string(Mix_GetError());
		}

	}

	void AudioSystem::processEntity(artemis::Entity & e)
	{
		AudioComponent* sound = audioMapper.get(e);
		if (sound->play) {
			Mix_PlayChannel(-1, audioMapper.get(e)->chunk, 0);
			sound->play = false;
		}
	}

}