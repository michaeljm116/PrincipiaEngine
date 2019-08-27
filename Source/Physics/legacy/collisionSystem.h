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
	CollisionSystem();
	void initialize();
	void processEntity(artemis::Entity& e);
};