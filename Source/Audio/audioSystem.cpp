#include "audioSystem.h"
#include <efx-creative.h>
#include <efx.h>
#include <al.h>
#include <alc.h>


AudioSystem::AudioSystem()
{
}

AudioSystem::~AudioSystem()
{
}

void AudioSystem::initialize()
{
	// Init openAL
	//alc
	//alutunlo
	//alutInit(0, NULL);
	// Clear Error Code (so we can catch any new errors)
	alGetError();
	
	alcOpenDevice(NULL);
}

void AudioSystem::processEntity(artemis::Entity & e)
{
}

void AudioSystem::updateListener(glm::vec3 pos)
{
}
