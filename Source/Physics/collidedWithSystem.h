#pragma once
#include "Artemis/Entity.h"
#include "Artemis/ComponentMapper.h"
#include "Artemis/EntityProcessingSystem.h"
#include "Components/collisionComponent.h"

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
	};
}
