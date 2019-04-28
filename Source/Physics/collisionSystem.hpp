#pragma once
/*
Collision System _H_ by Mike Murrell 3/1/2019
This system manages the behavor of all objects
with a collision component. Physics system also
does collisions but this is for specifically
chosen entities to perform game-specific tasks

edit: this might dramatically change once i get bvh's working
might just do my own collision detection instead.

editedit: OKAY so now  what this does is basically manage the 
state of the collisions so that you have a regular collisions list
you have an enter list so that on a colliion entering, it is activated
and an exit list so that when a collision is exited it is activated

upon activation
...nvm....
*/

#include "physicsSystem.h"

class CollisionSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<CollisionComponent> colMapper;
	artemis::ComponentMapper<RigidBodyComponent> rbMapper;

	artemis::EntityManager* em;

public:
	CollisionSystem(){
		addComponentType<CollisionComponent>();
		addComponentType<RigidBodyComponent>();
	}
	~CollisionSystem() {};

	void initialize() {
		colMapper.init(*world);
		rbMapper.init(*world);
	}
	void processEntity(artemis::Entity& e) {
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

};