#pragma once
#include "Components/immovableComponent.h"
#include "Components/collisionComponent.h"
#include "../Utility/transformComponent.hpp"
#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>

namespace Principia {

	const float EPSILON = 1.01f;

	class ImmovableSystem : public artemis::EntityProcessingSystem {
	public:
		ImmovableSystem();
		~ImmovableSystem();

		void initialize() override;
		void added(artemis::Entity &e) override;
		void processEntity(artemis::Entity &e) override;
		void removed(artemis::Entity &e) override;


	private:
		artemis::ComponentMapper<ImmovableComponent> imvMapper;
		artemis::ComponentMapper<CollisionComponent> collMapper;
		artemis::ComponentMapper<CollidedComponent > coldMapper;
		artemis::ComponentMapper<TransformComponent> transMapper;
	};
}