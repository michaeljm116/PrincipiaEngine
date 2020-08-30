
//#include "../pch.h"
#include <unordered_set>
#include <unordered_map>
#include <memory_resource>
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
	sys_Animate = (AnimateSystem*)world->getSystemManager()->getSystem<AnimateSystem>();
}

void Principia::AnimationSystem::processEntity(artemis::Entity & e)
{
	auto* ac = animMapper.get(e);
	if (ac->trans != 0) {
		if (ac->transTime == 0.f)
			transition(e);
		else
			ac->transTime += world->getDelta();
		if (ac->transTime > ac->time) {
			ac->start = ac->trans;
			ac->end = ac->transEnd;
			ac->trans = 0;
			ac->transEnd = 0;
			added(e);
		}
	}
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
			a->end = p.second;
			a->start = ((TransformComponent*)bfg->nodes[p.first]->data->getComponent<TransformComponent>())->local;
			bfg->nodes[p.first]->data->addComponent(a);
			bfg->nodes[p.first]->data->refresh();
		}
	}
	/* This needs to be done a little differently since lets say...
	 * Start = 1,5,7, End = 2,5,7. You want Children 1,2,5,7 to be called once 
	 * But you also want 1 5 7 to be 1st 5se 7se
	 * And you also want 2 5 7 to be 2te 5se 7se
	 * t = original transform, s = start e = end
	 * */
	else {
		std::unordered_map<int, AnimateComponent*> comps;
		// breakpoint check for walk: ac->start == -1164222069 && ac->end == -1142104506
		auto& startPose = RESOURCEMANAGER.getPose(ac->prefabName, ac->start);

		// Just straight up insert all the starts
		for (auto p : startPose.pose) {
			AnimateComponent* a = new AnimateComponent();
			a->flags = ac->flags;
			a->flags.startSet = 1;
			a->time = ac->time;
			a->start = p.second;
			comps.insert(std::make_pair(p.first, a));
		}

		// For the endFirst make sure there's no duplicates, then insert
		for (auto& p : endPose.pose) {
			const auto& a = comps.find(p.first);
			if (a != comps.end()) {
				a->second->end = p.second;
				a->second->flags.endSet = 1;
			}
			else {
				AnimateComponent* an = new AnimateComponent();
				an->flags = ac->flags;
				an->flags.endSet = 1;
				an->time = ac->time;
				an->end = p.second;
				comps.insert(std::make_pair(p.first, an));
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

	// make sure it not in trans
	ac->transTime = 0.001f;
}

//On remove, it makes sure all the animate components are also removed
void Principia::AnimationSystem::preRemoved(artemis::Entity & e)
{
	AnimationComponent* ac = animMapper.get(e);
	BFGraphComponent* bfg = bfgMapper.get(e);

	//First remove the endpose
	auto& endPose = RESOURCEMANAGER.getPose(ac->prefabName, ac->end);
	std::unordered_set<int> comp;
	for (const auto& p : endPose.pose) {
		//((TransformComponent*)bfg->nodes[p.second]->data->getComponent<TransformComponent>())->local = bfg->transforms[p.second];
		bfg->nodes[p.first]->data->preRemoveComponent<AnimateComponent>();
		comp.insert(p.first);
	}
	//Then if there's a start pose remove that too
	if (ac->num > 1) {
		auto& startPose = RESOURCEMANAGER.getPose(ac->prefabName, ac->start);
		for (const auto& p : startPose.pose) {
			if (comp.find(p.first) != comp.end()) {
				bfg->nodes[p.first]->data->preRemoveComponent<AnimateComponent>();
			}
		}
	}

	e.removeComponent<AnimationComponent>();
	e.refresh();
	change(e);
}

using cltuple = std::pair<int, std::tuple<Principia::sqt, Principia::sqt, Principia::AnimFlags>>;
//On Transition, unused parts go back to normal, and similar parts transition
void Principia::AnimationSystem::transition(artemis::Entity& e)
{
	auto* ac = animMapper.get(e);
	auto* bfg = bfgMapper.get(e);
	auto& startPose = RESOURCEMANAGER.getPose(ac->prefabName, ac->start);
	auto& endPose	= RESOURCEMANAGER.getPose(ac->prefabName, ac->end);
	auto& transPose = RESOURCEMANAGER.getPose(ac->prefabName, ac->trans);

	//First place every Previous Pose in a hashset
	//std::byte stackBuff[2048];
	//std::pmr::monotonic_buffer_resource rsrc(stackBuff, sizeof stackBuff);
	//auto prevPose = std::pmr::unordered_set<int>(0, &rsrc);// (startPose.pose.begin()->first, startPose.pose.end()->first);
	std::unordered_set<int> prevPose;

	for (auto& s : startPose.pose)	prevPose.insert(s.first);
	for (auto& e : endPose.pose)	prevPose.insert(e.first);

	//Create a list that combines everything
	//auto combinedList = std::pmr::unordered_map<int, std::tuple<sqt, sqt, AnimFlags>>(0, &rsrc);
	std::unordered_map<int, std::tuple<sqt, sqt, AnimFlags>> combinedList;

	//Go through the previous pose, Start = It's Transform, End = It's Original Transform
	for (auto& p : prevPose)
		combinedList.insert(cltuple(p, std::tuple<sqt, sqt, AnimFlags>(
		((TransformComponent*)bfg->nodes[p]->data->getComponent<TransformComponent>())->local,
		bfg->transforms[p], AnimFlags(0, 0, 1, 0))));

	//Go through the transitional pose, End = the transitional pose, 
	//Start = original transform if its not in list, or prevpose start if in the list
	for (auto& p : transPose.pose) {
		if (combinedList.find(p.first) == combinedList.end())
			combinedList.insert(cltuple(p.first, std::tuple<sqt, sqt, AnimFlags>(bfg->transforms[p.first], p.second, AnimFlags(0, 0, 1, 1))));
		else
			std::get<1>(combinedList[p.first]) = p.second;
			//combinedList[p.first].second = p.second;
	}

	//Iterate through the list and dispatch animate components
	for (auto& t : combinedList) {
		AnimateComponent* an = new AnimateComponent();
		an->flags = ac->flags;
		an->flags.endSet = 1;
		an->time = ac->time;
		an->start = std::get<0>(t.second);// .first;
		an->end = std::get<1>(t.second);// .second;
		an->flags = std::get<2>(t.second);
		
		bfg->nodes[t.first]->data->addComponent(an);
		bfg->nodes[t.first]->data->refresh();
	}

	//Turn off transform;
	ac->transTime = 0.0001f;

}
