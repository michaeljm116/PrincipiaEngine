#pragma once
/* Animation System Copyright (C) by Mike Murrell 2020
 * Hopefully the final revamp (prolly not tho lol) */

#include "animationComponent.hpp"
#include <Artemis/ComponentMapper.h>
#include <Artemis/EntityProcessingSystem.h>

//#include "../Utility/memory-arena.h"

namespace Principia {
	class AnimationSystem : public artemis::EntityProcessingSystem
	{
	public:
		AnimationSystem();
		~AnimationSystem();

		void initialize() override;
		void processEntity(artemis::Entity& e) override;
		void added(artemis::Entity& e) override;
		void preRemoved(artemis::Entity& e) override;

		//Timer t = Timer("Timer");
		//void begin() override { t.Start(); };
		//void end() override { t.End(); std::cout << t.ms << "ms\n"; };

	private:
		artemis::ComponentMapper<AnimationComponent> animMapper;
		artemis::ComponentMapper<BFGraphComponent> bfgMapper;

		void transition(artemis::Entity& e);
	};

}