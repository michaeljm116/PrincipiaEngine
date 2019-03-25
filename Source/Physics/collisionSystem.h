/*
Collision System _H_ by Mike Murrell 3/1/2019
This system manages the behavor of all objects 
with a collision component. Physics system also
does collisions but this is for specifically 
chosen entities to perform game-specific tasks

edit: this might dramatically change once i get bvh's working
might just do my own collision detection instead.
*/

#pragma once
#include "physicsSystem.h"

class CollisionSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<CollisionComponent> colMapper;
	artemis::ComponentMapper<RigidBodyComponent> rbMapper;
	artemis::ComponentMapper<NodeComponent> nodeMapper;
	artemis::ComponentMapper<SpringComponent> springMapper;
	//SpringMapper
	//ButtonMapper

	artemis::EntityManager* em;
	
public:
	CollisionSystem();
	~CollisionSystem();

	void initialize();
	void processEntity(artemis::Entity& e);

};
