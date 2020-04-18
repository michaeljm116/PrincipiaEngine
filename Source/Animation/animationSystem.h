#pragma once
/* Animation System Copyright (C) by Mike Murrell 2020
 * Hopefully the final revamp (prolly not tho lol) */

#include "../Utility/nodeComponent.hpp"
#include "animationComponent.hpp"
#include <Artemis/ComponentMapper.h>

namespace Principia {
	class AnimationSystem : public artemis::EntityProcessingSystem
	{
	public:
		AnimationSystem();
		~AnimationSystem();

		void initialize();
		void processEntity(artemis::Entity& e);
		void added(artemis::Entity& e);
		void removed(artemis::Entity& e);

	private:
		artemis::ComponentMapper<AnimationComponent> animMapper;
		artemis::ComponentMapper<BFGraphComponent> bfgMapper;
	};

}