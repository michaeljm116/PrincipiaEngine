#pragma once

#include "audioComponents.h"
//#include <alc.h>

namespace Principia {
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
}