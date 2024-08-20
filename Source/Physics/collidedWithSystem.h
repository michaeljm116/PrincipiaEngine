#pragma once
#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>
#include "Components/collisionComponent.h"
#include "../Utility/nodeComponent.hpp"
#include "../Utility/script-component.h"

namespace Principia {
	class CollidedWithSystem : public artemis::EntityProcessingSystem {
	public:
		CollidedWithSystem();
		~CollidedWithSystem();

		void initialize() override;
		void added(artemis::Entity &e) override;
		void removed(artemis::Entity &e) override;
		void processEntity(artemis::Entity &e) override;

	private:
		artemis::ComponentMapper<CollidedComponent> cwMapper;
		artemis::ComponentMapper<NodeComponent> nodeMapper;
		artemis::ComponentMapper<Cmp_Collision_Script> scriptMapper;
	};
}
