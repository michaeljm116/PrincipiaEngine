#pragma once
#include "collisionSystem.h"

CollisionSystem::CollisionSystem() {
	addComponentType<CollisionComponent>();
	addComponentType<RigidBodyComponent>();
}

void CollisionSystem::initialize() {
	colMapper.init(*world);
	rbMapper.init(*world);
}

void CollisionSystem::processEntity(artemis::Entity& e) {
	CollisionComponent* cc = colMapper.get(e);
	std::vector<int> erase;
	for (auto col = cc->collisions.begin(); col != cc->collisions.end(); ++col) {
		switch (col->second.state)
		{
		case CollisionState::Exit:
			erase.push_back(col->first);
			break;
		case CollisionState::Continue:
			col->second.state = CollisionState::Exit;
			break;
		case CollisionState::Start:
			col->second.state = CollisionState::Continue;
			break;
		default:
			break;
		}
	}

	if (erase.size() > 0) {
		for (int i = 0; i < erase.size(); ++i) {
			cc->collisions.erase(erase[i]);
		}
	}
	//for (auto col : cc->collisions) {
	//	artemis::Entity& colider = em->getEntity(col.first);


	//	if (nc->flags & 1) {
	//		//If you collided with a spring, take the springs normal and power and make an impulse with it
	//		e.addComponent(new ImpulseComponent(springMapper.get(colider)->normal, springMapper.get(colider)->power));
	//		e.refresh();
	//		/*so the question is whats the difference between ball collision and just collision???*/
	//	}
	//}
}