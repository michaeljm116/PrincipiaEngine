#pragma once

#include "../Utility/componentIncludes.h"
#include "../Game/Gameplay/characterComponent.hpp"
#include "audioComponents.h"
//#include <alc.h>

class AudioSystem : public artemis::EntityProcessingSystem
{
private:
	artemis::ComponentMapper<AudioComponent> audioMapper;
public:
	AudioSystem();
	~AudioSystem();
	void added(artemis::Entity& e);
	void initialize();
	void processEntity(artemis::Entity& e);

};
