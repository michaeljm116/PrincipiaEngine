#include "collisionSystem.h"

CollisionSystem::CollisionSystem()
{
	addComponentType<CollisionComponent>();
	addComponentType<RigidBodyComponent>();
	addComponentType<NodeComponent>(); 
}

CollisionSystem::~CollisionSystem()
{
}

void CollisionSystem::initialize()
{
}

void CollisionSystem::processEntity(artemis::Entity & e)
{
	CollisionComponent* cc = colMapper.get(e);
	NodeComponent* nc = nodeMapper.get(e);
	for (auto col : cc->collisions) {
		artemis::Entity& colider = em->getEntity(col.first);


		if(nc->flags & 1) {
			//If you collided with a spring, take the springs normal and power and make an impulse with it
			e.addComponent(new ImpulseComponent(springMapper.get(colider)->normal, springMapper.get(colider)->power));
			e.refresh();
			//physissystemchange
		}
	}

}
