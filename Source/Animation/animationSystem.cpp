
//#include "../pch.h"
#include <unordered_map>
#include "animationSystem.h"
#include "../Utility/resourceManager.h"


Principia::AnimationSystem::AnimationSystem()
{
	addComponentType<AnimationComponent>();
	addComponentType<BFGraphComponent>();
}

Principia::AnimationSystem::~AnimationSystem()
{
}

void Principia::AnimationSystem::initialize()
{
	animMapper.init(*world);
	bfgMapper.init(*world);
}

void Principia::AnimationSystem::processEntity(artemis::Entity & e)
{
}

void Principia::AnimationSystem::added(artemis::Entity & e)
{
	AnimationComponent* ac = animMapper.get(e);
	BFGraphComponent* bfg = bfgMapper.get(e);

	//If there's only 1 pose, then it'll only be the end pose 
	auto& endPose = RESOURCEMANAGER.getPose(ac->prefabName, ac->end);
	if (ac->num == 1) {
		for (auto p : endPose.pose) {
			AnimateComponent* a = new AnimateComponent();
			a->flags = ac->flags;
			a->time = ac->time;
			a->end = p.first;
			a->start = ((TransformComponent*)bfg->nodes[p.second]->data->getComponent<TransformComponent>())->local;
			bfg->nodes[p.second]->data->addComponent(a);
			bfg->nodes[p.second]->data->refresh();
		}
	}
	/* This needs to be done a little differently since lets say...
	 * Start = 1,5,7, End = 2,5,7. You want Children 1,2,5,7 to be called once 
	 * But you also want 1 5 7 to be 1st 5se 7se
	 * And you also want 2 5 7 to be 2te 5se 7se
	 * t = original transform, s = start e = end
	 */
	else {
		std::unordered_map<int, AnimateComponent*> comps;
		auto& startPose = RESOURCEMANAGER.getPose(ac->prefabName, ac->start);

		// Just straight up insert all the starts
		for (auto p : startPose.pose) {
			AnimateComponent* a = new AnimateComponent();
			a->flags = ac->flags;
			a->flags.startSet = 1;
			a->time = ac->time;
			a->start = p.first;
			comps.insert(std::make_pair(p.second, a));
		}

		// For the endFirst make sure there's no duplicates, then insert
		for (auto& p : endPose.pose) {
			auto& a = comps.find(p.second);
			if (a != comps.end()) {
				a->second->end = p.first;
				a->second->flags.endSet = 1;
			}
			else {
				AnimateComponent* an = new AnimateComponent();
				an->flags = ac->flags;
				an->flags.endSet = 1;
				an->time = ac->time;
				an->end = p.first;
				comps.insert(std::make_pair(p.second, an));
			}
		}

		// Now dispatch the components
		for (auto& c : comps) {
			auto* ent = bfg->nodes[c.first]->data;
			auto* tc = (TransformComponent*)ent->getComponent<TransformComponent>();
			
			if (c.second->flags.startSet == 0) 
				c.second->start = tc->local;
			if (c.second->flags.endSet == 0) 
				c.second->end = tc->local;

			ent->addComponent(c.second);
			ent->refresh();
		}
	}
}

void Principia::AnimationSystem::removed(artemis::Entity & e)
{
}
