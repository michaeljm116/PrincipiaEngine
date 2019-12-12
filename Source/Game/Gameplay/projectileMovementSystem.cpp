#include "projectileMovementSystem.h"

//#include "../scene.h"
//using namespace Principia;
ProjectileMovementSystem::ProjectileMovementSystem()
{
	addComponentType<ProjectileComponent>();
	addComponentType<TransformComponent>();
	addComponentType<Principia::CollisionComponent>();
}

ProjectileMovementSystem::~ProjectileMovementSystem()
{
}

void ProjectileMovementSystem::initialize()
{
	projMapper.init(*world);
	transMapper.init(*world);
	colMapper.init(*world);
}

void ProjectileMovementSystem::removed(artemis::Entity & e)
{
	world->getEntityManager()->remove(e);
}

void ProjectileMovementSystem::processEntity(artemis::Entity & e)
{
	ProjectileComponent* p = projMapper.get(e);
	TransformComponent* t = transMapper.get(e);
	t->local.position += p->dir * p->speed * world->getDelta();

	//if you collide remove
	//if (colMapper.get(e)->state == Principia::CollisionState::Start) {
	//	e.removeComponent<ProjectileComponent>();
	//	e.refresh();
	//	change(e);
	//}

	//if you exist for more than 
	if (t->local.position.z > p->maxHeight) {
		e.removeComponent<ProjectileComponent>();
		e.refresh();
		change(e);
	}
}
