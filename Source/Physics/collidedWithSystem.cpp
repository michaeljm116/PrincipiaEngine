#include "../pch.h"
#include "collidedWithSystem.h"
#include <algorithm>
#pragma once

Principia::CollidedWithSystem::CollidedWithSystem()
{
	addComponentType<CollidedComponent>();
	addComponentType<NodeComponent>();
	addComponentType<Cmp_Collision_Script>();
}

Principia::CollidedWithSystem::~CollidedWithSystem()
{
}

void Principia::CollidedWithSystem::initialize()
{
	cwMapper.init(*world);
	nodeMapper.init(*world);
	scriptMapper.init(*world);
}

void Principia::CollidedWithSystem::added(artemis::Entity & e)
{
	//e.removeComponent<CollidedComponent>();
	CollidedComponent* cc = cwMapper.get(e);
	for (auto& cw : cc->collidedWith) {
		if (cw.state == CollisionState::None) {
			cw.state = CollisionState::Enter;

			auto* node = nodeMapper.get(e);
			auto* cn = (NodeComponent*)world->getEntity(cw.id).getComponent<NodeComponent>();
			std::cout << "Collision Begin: " << node->name << " | " << cn->name << "\n";
		}
	}
	scriptMapper.get(e)->OnCollisionEnter(cc);
}

void Principia::CollidedWithSystem::removed(artemis::Entity & e)
{
	world->getEntityManager()->removeComponent<CollidedComponent>(e);
	e.refresh();
}

void Principia::CollidedWithSystem::processEntity(artemis::Entity & e)
{ 
	std::vector<CollisionData>& cw = cwMapper.get(e)->collidedWith;
	auto* node = nodeMapper.get(e);
	auto* script = scriptMapper.get(e);
	std::vector<CollisionData> copy = cw;

	//Remove anything that's no longer colliding
	cw.erase(std::remove_if(cw.begin(), cw.end(), [&](CollisionData& cd) {
		if (cd.state == CollisionState::None) {
			cd.state = CollisionState::Enter;
			return false;
		}

		cd.timer -= 1; 
		//int prev = cd.prev;
		//cd.prev = cd.timer;
		//return cd.timer <= prev;
		bool removed = cd.timer <= 0;
		if (removed) {
			auto* e = &world->getEntity(cd.id);
			if (e != nullptr) {
				script->OnCollisionExit(e);
				auto* n = (NodeComponent*)world->getEntity(cd.id).getComponent<NodeComponent>();
				std::cout << "Collision End: " << node->name << " | " << n->name << "\n";
			}
		}
		return removed;
	}), cw.end());

	//if no collisions then empty
	if (cw.empty()) {
		////auto* node = (NodeComponent*)world->getEntityManager()->getComponent<NodeComponent*>(e);
		//if (node != nullptr) {
		//	std::cout << "Collision for: " << node->name << " is removed!\n";/*
		//	std::cout << "Copy Data: \n";
		//	for (auto c : copy) {
		//		auto* n = (NodeComponent*)world->getEntity(c.id).getComponent<NodeComponent>();
		//		std::cout << "\t" << n->name << "\n";
		//	}*/
		//}
		e.removeComponent<CollidedComponent>();
		e.refresh();
	}
	else
		// otherwise Process the collisions
		script->OnCollisionStay(world->getGameTick());

}

