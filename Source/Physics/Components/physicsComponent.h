#pragma once
#include <Artemis/Component.h>

template <typename T>
class btAlignedObjectArray;
class btCollisionShape;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btManifoldPoint;

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