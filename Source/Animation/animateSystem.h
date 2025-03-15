#pragma once
/* Animate System Copyright (C) by Mike Murrell 2020
 * Takes in an Animate & Transform Component
 * And Interpolates the animation */

#include <Artemis/ComponentMapper.h>
#include <Artemis/ImmutableBag.h>
#include "../Utility/timer.hpp"

namespace Principia {
	struct TransformComponent;
	struct AnimateComponent;

	class AnimateSystem : public artemis::EntityProcessingSystem {
	public:
		AnimateSystem();
		~AnimateSystem();
		void initialize() override;
		void added(artemis::Entity &e) override;
		void processEntity(artemis::Entity &e) override;
		void processEntities(artemis::ImmutableBag<artemis::Entity*>& bag) override;
		void preRemoved(artemis::Entity &e) override;
		//void begin() override { AnimationsTimer.Start(); }
		//void end() override { AnimationsTimer.End(); std::cout << "\nAnim: " << AnimationsTimer.ms; };
	private:
		artemis::ComponentMapper<TransformComponent> transMapper;
		artemis::ComponentMapper<AnimateComponent> animMapper;

		inline bool CheckIfFinished(const sqt & curr, AnimateComponent * const ac);
		Principia::Timer AnimationsTimer = Principia::Timer("Animation");

		
	};

}