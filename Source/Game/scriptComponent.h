#pragma once
#include <Artemis/Artemis.h>

class ScriptComponent : public artemis::Component {
	virtual void Start() = 0;
	virtual void Update(float dt) = 0;
	virtual void End() = 0;

};

