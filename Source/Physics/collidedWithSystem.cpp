#include "collidedWithSystem.h"
#include <algorithm>
#pragma once

Principia::CollidedWithSystem::CollidedWithSystem()
{
	addComponentType<CollidedComponent>();
}

Principia::CollidedWithSystem::~CollidedWithSystem()
{
}

void Principia::CollidedWithSystem::initialize()
{
	cwMapper.init(*world);
}

void Principia::CollidedWithSystem::added(artemis::Entity & e)
{
	//e.removeComponent<CollidedComponent>();
	CollidedComponent* cc = cwMapper.get(e);
	for (auto& cw : cc->collidedWith) {
		if (cw.state == CollisionState::None)
			cw.state = CollisionState::Enter;
	}
}

void Principia::CollidedWithSystem::removed(artemis::Entity & e)
{
	world->getEntityManager()->removeComponent<CollidedComponent>(e);
	e.refresh();
}

void Principia::CollidedWithSystem::processEntity(artemis::Entity & e)
{ 
	//std::vector<CollisionData>& cw = cwMapper.get(e)->collidedWith;
	//cw.erase(std::remove_if(cw.begin(), cw.end(), [](CollisionData& cd) {
	//	cd.timer -= 1; 
	//	int prev = cd.prev;
	//	cd.prev = cd.timer;
	//	return cd.timer == prev;
	//}), cw.end());

	//if (cw.empty()) {
	//	e.removeComponent<CollidedComponent>();
	//	e.refresh();
	//}
}

