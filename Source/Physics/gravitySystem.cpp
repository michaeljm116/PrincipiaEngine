#include "../pch.h"
#include "gravitySystem.h"
#pragma once

Principia::SysGravity::SysGravity()
{
	addComponentType<CmpGravity>();
	addComponentType<CollisionComponent>();
	addComponentType<TransformComponent>();
}

Principia::SysGravity::~SysGravity()
{
}

void Principia::SysGravity::initialize()
{
	gravMapper.init(*world);
	transMapper.init(*world);
	collMapper.init(*world);
}

void Principia::SysGravity::added(artemis::Entity & e)
{
	//Load the tuple
	auto* tc = transMapper.get(e);
	auto* cc = collMapper.get(e);
	auto* gc = gravMapper.get(e);

	//Set the difference
	gc->diff = cc->extents.y + ground;

	//ground it if its lower already
	if (tc->local.position.y < gc->diff) {
		tc->local.position.y = gc->diff;
		gc->grounded = true;
	}
}

void Principia::SysGravity::processEntity(artemis::Entity & e)
{
	//Load the tuple
	auto* tc = transMapper.get(e);
	auto* gc = gravMapper.get(e);

	//lower it and check if grounded
	tc->local.position.y -= gc->gforce * world->getGameTick();
	if (tc->local.position.y < gc->diff) {
		tc->local.position.y = gc->diff;
		gc->grounded = true;
	}
}

void Principia::SysGravity::removed(artemis::Entity & e)
{
}
