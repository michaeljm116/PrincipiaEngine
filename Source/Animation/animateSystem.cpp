#include "animateSystem.h"
#define GLM_FORCE_SSE2
#include <glm/simd/common.h>
#include <glm/gtc/epsilon.hpp>
Principia::AnimateSystem::AnimateSystem()
{
	addComponentType<TransformComponent>();
	addComponentType<AnimateComponent>();
}

Principia::AnimateSystem::~AnimateSystem()
{
}

void Principia::AnimateSystem::initialize()
{
	transMapper.init(*world);
	animMapper.init(*world);
}

void Principia::AnimateSystem::added(artemis::Entity & e)
{
	//Onadd you can choose to reset the animation to the start
	//Or just interpolate from where you're already at
	AnimateComponent* ac = animMapper.get(e);
	TransformComponent* tc = transMapper.get(e);

	//This forces the animation to go to the start position
	if (ac->flags.forceStart == 1) 
		tc->local = ac->start;

	//Initial check (so you dont have to process unused data)
	CheckIfFinished(tc->local, ac);
}

void Principia::AnimateSystem::processEntity(artemis::Entity & e)
{	
	AnimateComponent* ac = animMapper.get(e);
	//if (!ac) 
	//	return;
	TransformComponent* tc = transMapper.get(e);
	auto delta =  world->getDelta();
	if (delta > 1) delta = 0.1f;
	auto dt = glm::vec4(delta/ac->time);
	ac->currTime += world->getDelta();

	//Interpolate dat ish
	if(!ac->flags.pf)tc->local.position = glm::mix(tc->local.position, ac->end.position, dt);
	if(!ac->flags.sf)tc->local.scale	= glm::mix(tc->local.scale, ac->end.scale, dt);
	if(!ac->flags.rf)tc->local.rotation = glm::lerp(tc->local.rotation, ac->end.rotation, dt.x);

	//End Animation if finished
	/*if (CheckIfFinished(tc->local, ac)) {
		ac->flags.pf = 0; ac->flags.rf = 0; ac->flags.sf = 0;
		if (ac->flags.loop == 1)
			std::swap(ac->end, ac->start);
		else
			e.removeComponent<AnimateComponent>();
	}*/
	if (ac->currTime >= ac->time) {
		ac->currTime = 0.f;
		if(ac->flags.forceEnd == 1)
			tc->local = ac->end;
		if (ac->flags.loop == 1) {
			std::swap(ac->end, ac->start);
			auto n = (NodeComponent*)e.getComponent<NodeComponent>();
			std::cout << "\nLooped: " << n->name;
		}
		else
			e.preRemoveComponent<AnimateComponent>();
	}

}

void Principia::AnimateSystem::preRemoved(artemis::Entity & e)
{
	AnimateComponent* ac = animMapper.get(e);
	if(ac->flags.forceEnd == 1) transMapper.get(e)->local = ac->start;
	e.removeComponent<AnimateComponent>();
	e.refresh();
	change(e);
}

//This sets the finish flags and returns true if they've all finished transforming
inline bool Principia::AnimateSystem::CheckIfFinished(const sqt & curr, AnimateComponent * const ac)
{

	auto ep = 0.01f;
	auto epv = glm::vec4(ep);
	glm::bvec4 p = glm::epsilonEqual(curr.position, ac->end.position, ep);
	glm::bvec4 r = glm::epsilonEqual(curr.rotation, ac->end.rotation, 0.2f);
	glm::bvec4 s = glm::epsilonEqual(curr.scale, ac->end.scale, ep);

	ac->flags.pf = p.x & p.y & p.z;
	ac->flags.rf = r.x & r.y & r.z;
	ac->flags.sf = s.x & s.y & s.z;
	uint_fast8_t animFinished = (ac->flags.pf | (ac->flags.rf << 1) | (ac->flags.sf << 2));

	return (animFinished == 7);
}

