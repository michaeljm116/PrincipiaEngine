#include "../pch.h"
#include "script-system.h"
#include "script-component.h"

Principia::Sys_Scripting::Sys_Scripting()
{
	addComponentType<ScriptComponent>();
}

Principia::Sys_Scripting::~Sys_Scripting()
{
}

void Principia::Sys_Scripting::initialize()
{
	scriptMapper.init(*world);
}

void Principia::Sys_Scripting::added(artemis::Entity& e)
{
	scriptMapper.get(e)->added();
}

void Principia::Sys_Scripting::removed(artemis::Entity& e)
{
	//scriptMapper.get(e)->removed();
}

void Principia::Sys_Scripting::processEntity(artemis::Entity& e)
{
	scriptMapper.get(e)->process(world->getGameTick());
}
