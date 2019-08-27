#include "ballMovementSystem.h"

BallMovementSystem::BallMovementSystem() {
	addComponentType<BallComponent>();
	addComponentType<RigidBodyComponent>();
}

void BallMovementSystem::initialize() {
	ballMapper.init(*world);
	rbMapper.init(*world);
}

void BallMovementSystem::processEntity(artemis::Entity &e) {
	BallComponent*		ball = ballMapper.get(e);
	RigidBodyComponent* rbc = rbMapper.get(e);

	btTransform& trans = rbc->body->getWorldTransform();
	btVector3 pos = trans.getOrigin();
	float delta = world->getDelta();
	//if (delta < 1.f) {
		pos += glm2bulVec3(ball->direction * ball->speed * world->getDelta());
		trans.setOrigin(pos);
	//}
}

btVector3 BallMovementSystem::glm2bulVec3(const glm::vec3& in) {
	return btVector3(in[0], in[1], in[2]);
}