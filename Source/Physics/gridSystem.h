#pragma once
/*
Mike Murrell 2019 Grid System
Any time a static object is created it's placed on a grid
*/

#include "Artemis/Artemis.h"
#include "Components/staticComponent.h"
#include "Components/collisionComponent.h"
#include "Components/gridComponent.h"

namespace Principia {
	class GridSystem : public artemis::EntityProcessingSystem {
	public:
		GridSystem();
		~GridSystem();

		void initialize() override;
		void processEntity(artemis::Entity &e) override;
		void added(artemis::Entity &e) override;
		void removed(artemis::Entity &e) override;
		void printGrid();

	private:
		artemis::ComponentMapper<StaticComponent> staticMapper;
		artemis::ComponentMapper<CollisionComponent> colMapper;
		GridComponent* gridComp;
	};
}