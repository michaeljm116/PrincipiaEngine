#pragma once
#include "Components/collisionComponent.h"
#include "Artemis/EntityProcessingSystem.h"
#include "Artemis/ComponentMapper.h"
#include "Artemis/Entity.h"
#include "../Game/Gameplay/Components/gameObjectTypeComponent.h"
#include <set>

namespace Principium {
	class CollisionSystem : public artemis::EntityProcessingSystem {
	public:
		CollisionSystem();
		~CollisionSystem();

		void initialize();
		void begin();
		void end();
		void added(artemis::Entity &e);
		void removed(artemis::Entity &e);

		void checkCollision(artemis::Entity& a, artemis::Entity& b);
		
	private:
		artemis::ComponentMapper<CollisionComponent> colMapper;
		artemis::ComponentMapper<GameObjectTypeComponent> gotMapper;

		std::set<artemis::Entity*> players;
		std::set<artemis::Entity*> enemies;

	};
}