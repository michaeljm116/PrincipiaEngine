#include "../pch.h"
#include "immovableSystem.h"

Principia::ImmovableSystem::ImmovableSystem()
{
	addComponentType<ImmovableComponent>();
	addComponentType<CollidedComponent>();
	addComponentType<TransformComponent>();
	addComponentType<CollisionComponent>();
}

Principia::ImmovableSystem::~ImmovableSystem()
{
}

void Principia::ImmovableSystem::initialize()
{
	imvMapper.init(*world);
	collMapper.init(*world);
	coldMapper.init(*world);
	transMapper.init(*world);
}

void Principia::ImmovableSystem::added(artemis::Entity & e)
{
	CollidedComponent* col = coldMapper.get(e);
	for (auto cw : col->collidedWith) {
		artemis::Entity* ent = &world->getEntity(cw.id);
		TransformComponent* tc = (TransformComponent*)ent->getComponent<TransformComponent>();
		tc->local.position = glm::vec4(cw.colpoint + (EPSILON * cw.normal), 0.f);
	}
	//e.removeComponent<CollidedComponent>();
}

void Principia::ImmovableSystem::processEntity(artemis::Entity & e)
{
	//TransformComponent* a = transMapper.get(e);
	//auto vec = colMapper.get(e)->collidedWith;
	CollidedComponent* colc = coldMapper.get(e);
	CollisionComponent* cc = collMapper.get(e);
	if (colc != nullptr && cc != nullptr) {
		for (auto cw : colc->collidedWith) {
			//cw.position += cw.normal * world->getDelta() * 10.f;
			TransformComponent* tc = (TransformComponent*)world->getEntity(cw.id).getComponent<TransformComponent>();
			tc->local.position += glm::vec4(cw.normal * world->getGameTick() * 10.f, 0.f);
			glm::vec4 clamper = glm::vec4(cw.colpoint + (cw.normal * EPSILON), 0.f);
			//tc->local.position = cw.colpoint + (cw.normal * EPSILON);
			clamper.y = tc->local.position.y;
			tc->local.position = glm::clamp(tc->local.position, -clamper, clamper);
			//glm::vec3 bounds = 
		}
	}
	//TransformComponent* b = (TransformComponent*)colMapper.get(e)->collidedWith
}

void Principia::ImmovableSystem::removed(artemis::Entity & e)
{
}
