#include "animateSystem.h"
#define GLM_FORCE_SSE2
#define GLM_FORCE_ALIGN
#include <glm/simd/common.h>
#include <glm/gtc/epsilon.hpp>
//#include <omp.h>
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
	//Get the Components
	AnimateComponent*	ac = animMapper.get(e);
	TransformComponent* tc = transMapper.get(e);

	//Increment time
	auto x = glm::clamp(world->getDelta() / ac->time, 0.f, 1.f);
	glm::vec4  dt = glm::vec4(x);
	ac->currTime += world->getDelta();

	//Interpolate dat ish
	if(!ac->flags.pf)tc->local.position = glm::mix(tc->local.position, ac->end.position, dt);
	if(!ac->flags.sf)tc->local.scale	= glm::mix(tc->local.scale, ac->end.scale, dt);
	if(!ac->flags.rf)tc->local.rotation = glm::lerp(tc->local.rotation, ac->end.rotation, dt.x);

	/*if (CheckIfFinished(tc->local, ac)) {
		ac->flags.pf = 0; ac->flags.rf = 0; ac->flags.sf = 0;
		if (ac->flags.loop == 1)
			std::swap(ac->end, ac->start);
		else
			e.removeComponent<AnimateComponent>();
	}*/
	//End Animation if finished
	if (ac->currTime >= ac->time) {
		ac->currTime = 0.f;
		//if(ac->flags.forceEnd == 0)
		//	tc->local = ac->end;

		if (ac->flags.loop == 1 || ac->flags.forceEnd == 1)
			std::swap(ac->end, ac->start);
		else
			e.preRemoveComponent<AnimateComponent>();

		if (ac->flags.forceEnd == 1) {
			ac->flags.forceEnd = 0;
			//std::swap(ac->end, ac->start);
		}
	}

}

void Principia::AnimateSystem::processEntities(artemis::ImmutableBag<artemis::Entity*>& bag)
{
	//#pragma omp parallel for 
	for (int i = 0; i < bag.getCount(); ++i)
		processEntity(*bag.get(i));
}

void Principia::AnimateSystem::preRemoved(artemis::Entity & e)
{
	AnimateComponent* ac = animMapper.get(e);
	if(ac->flags.forceEnd == 1) transMapper.get(e)->local = ac->end;
	e.removeComponent<AnimateComponent>();
	e.refresh();
	change(e);
}

//This sets the finish flags and returns true if they've all finished transforming
inline bool Principia::AnimateSystem::CheckIfFinished(const sqt & curr, AnimateComponent * const ac)
{
	float ep = 0.01f;
	glm::bvec4 p = glm::epsilonEqual(curr.position, ac->end.position, ep);
	glm::bvec4 r = glm::epsilonEqual(curr.rotation, ac->end.rotation, ep);
	glm::bvec4 s = glm::epsilonEqual(curr.scale, ac->end.scale, ep);

	ac->flags.pf = p.x & p.y & p.z;
	ac->flags.rf = r.x & r.y & r.z;
	ac->flags.sf = s.x & s.y & s.z;
	uint_fast8_t animFinished = (ac->flags.pf | (ac->flags.rf << 1) | (ac->flags.sf << 2));

	return (animFinished == 7);
}

