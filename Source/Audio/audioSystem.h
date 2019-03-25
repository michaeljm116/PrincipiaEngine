#pragma once

#include "../Utility/componentIncludes.h"
#include <al.h>

class AudioSystem : public artemis::EntityProcessingSystem
{
public:
	AudioSystem();
	~AudioSystem();
	void initialize();
	void processEntity(artemis::Entity& e);
	void updateListener(glm::vec3 pos);
private:


};
