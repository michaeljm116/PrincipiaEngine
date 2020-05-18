#pragma once
/* Animate System Copyright (C) by Mike Murrell 2020
 * Takes in an Animate & Transform Component
 * And Interpolates the animation */

#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>
#include "../Utility/transformComponent.hpp"
#include "animationComponent.hpp"

namespace Principia {
	class AnimateSystem : public artemis::EntityProcessingSystem {
	public:
		AnimateSystem();
		~AnimateSystem();

		void initialize() override;
		void added(artemis::Entity &e) override;
		void processEntity(artemis::Entity &e) override;
		void preRemoved(artemis::Entity &e) override;

	private:
		artemis::ComponentMapper<TransformComponent> transMapper;
		artemis::ComponentMapper<AnimateComponent> animMapper;

		inline bool CheckIfFinished(const sqt & curr, AnimateComponent * const ac);
	};

}