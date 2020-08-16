#pragma once
#include "Components/collisionComponent.h"
#include "Components/dynamicComponent.h"
#include "Components/gridComponent.h"
#include "Artemis/EntityProcessingSystem.h"
#include "Artemis/ComponentMapper.h"
#include "Artemis/Entity.h"
//#include "../Game/Gameplay/Components/gameObjectTypeComponent.h"
#include "../Utility/transformComponent.hpp"
#include <set>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace Principia {
	class CollisionSystem : public artemis::EntityProcessingSystem {
	public:
		CollisionSystem();
		~CollisionSystem();

		void initialize();
		void begin();
		void processEntity(artemis::Entity &e) override;
		void added(artemis::Entity& e)override;
		void removed(artemis::Entity& e)override;

		void checkDynamicCollisions();
		void checkStaticCollision(artemis::Entity& e);
		void checkCollision(artemis::Entity& a, artemis::Entity& b);
		glm::vec3 rotateBounds(const glm::quat & m, const glm::vec3 & extents);
		
		void bulletInit();
		void bulletAdded(artemis::Entity& e);
		void bulletRemoved(artemis::Entity& e);
		void bulletUpdate();
		void bulletProcessEntity(artemis::Entity& e);
		void bulletOnCollision(btCollisionObject* a, btCollisionObject* b, btManifoldPoint p);

	private:
		artemis::ComponentMapper<CollisionComponent> colMapper;
		artemis::ComponentMapper<TransformComponent> transMapper;
		artemis::ComponentMapper<DynamicComponent> dynamicMapper;
		//artemis::ComponentMapper<GameObjectTypeComponent> gotMapper;

		GridComponent* grid;

		//Bullet
		btAlignedObjectArray<btCollisionShape*> collisionShapes;
		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btBroadphaseInterface* overlappingPairCache;
		btSequentialImpulseConstraintSolver* solver;
		btSimpleDynamicsWorld* dynamicsWorld;
		

	};
}