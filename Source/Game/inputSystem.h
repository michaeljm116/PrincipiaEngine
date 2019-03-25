#pragma once
/* Input System Copyright (C) by Mike Murrell 
*/
#include "../Utility/Input.h"
#include "inputComponent.hpp"
#include <Artemis/Artemis.h>

class InputSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<InputComponent> inputMapper;

public:
	InputSystem();
	~InputSystem();

	void initialize();
	void processEntity(artemis::Entity &e);
	void update();
	
};