#pragma once
#include "Components/collisionComponent.h"
#include "Components/gravityComponent.h"
#include "../Utility/transformComponent.hpp"
#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>

namespace Principia {

	class SysGravity : public artemis::EntityProcessingSystem {
	public:
		SysGravity();
		~SysGravity();

		void initialize() override;
		void added(artemis::Entity& e) override;
		void processEntity(artemis::Entity& e) override;
		void removed(artemis::Entity& e) override;


	private:
		artemis::ComponentMapper<CmpGravity> gravMapper;
		artemis::ComponentMapper<CollisionComponent> collMapper;
		artemis::ComponentMapper<TransformComponent> transMapper;
		
		float ground = -6.f;
	};

}