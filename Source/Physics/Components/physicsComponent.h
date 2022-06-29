#pragma once
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include "Artemis/Component.h"

namespace Principia {
	struct Cmp_Physics : public artemis::Component {
		btAlignedObjectArray<btCollisionShape*> collisionShapes;// = nullptr;
		btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
		btCollisionDispatcher* dispatcher = nullptr;
		btBroadphaseInterface* overlappingPairCache = nullptr;
		btSequentialImpulseConstraintSolver* solver = nullptr;
		//btSimpleDynamicsWorld* dynamicsWorld = nullptr;
		btDiscreteDynamicsWorld* dynamicsWorld = nullptr;
	};
}