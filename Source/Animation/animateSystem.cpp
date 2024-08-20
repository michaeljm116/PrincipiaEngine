#include "../pch.h"
#include "animateSystem.h"
#define GLM_FORCE_SSE2
#define GLM_FORCE_ALIGN
#include <glm/simd/common.h>
#include <glm/gtc/epsilon.hpp>
#include "../Utility/transformComponent.hpp"
#include "animationComponent.hpp"
#include <omp.h>

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
	if (&e == nullptr) 
		return;
	//Get the Components
	AnimateComponent*	ac = animMapper.get(e);
	TransformComponent* tc = transMapper.get(e);

	if (ac == nullptr) {
		std::cout << "ERROR NULL ANIM MAPPER ON " << e.getId();
		NodeComponent* nc = (NodeComponent*)e.getComponent<NodeComponent>();
		if (nc != nullptr) std::cout << " Name: " << nc->name << std::endl;	
		return;
	};
	if (tc == nullptr){
		std::cout << "ERROR NULL ANIM MAPPER ON " << e.getId();
		NodeComponent* nc = (NodeComponent*)e.getComponent<NodeComponent>();
		if (nc != nullptr) std::cout << " Name: " << nc->name << std::endl;
		return;
	}

	////Remove if parent is deleted
	//if (ac->parent == nullptr)
	//	e.removeComponent<AnimateComponent>();

	//Increment time
	auto x = glm::clamp(world->getGameTick() / ac->time, 0.f, 1.f);
	glm::vec4  dt = glm::vec4(x);
	ac->currTime += world->getGameTick();

	//Interpolate dat ish
	if(!ac->flags.pos_flag)tc->local.position = glm::mix(tc->local.position, ac->end.position, dt);
	if(!ac->flags.sca_flag)tc->local.scale	= glm::mix(tc->local.scale, ac->end.scale, dt);
	if(!ac->flags.rot_flag)tc->local.rotation = glm::slerp(tc->local.rotation, ac->end.rotation, dt.x);

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

	ac->flags.pos_flag = p.x & p.y & p.z;
	ac->flags.rot_flag = r.x & r.y & r.z;
	ac->flags.sca_flag = s.x & s.y & s.z;
	uint_fast8_t animFinished = (ac->flags.pos_flag | (ac->flags.rot_flag << 1) | (ac->flags.sca_flag << 2));

	return (animFinished == 7);
}

