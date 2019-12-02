/*
Character Rotation System
Mike Murrell 11/30/2019
A system that controls the rotation of characters
It looks at the axis of the character
and based off it will transform it 
according to 8 different directions
*/

#pragma once
#include "Components/chracterRotationComponent.h"
#include "../Application/controllerComponent.hpp"
#include "../Game/transformComponent.hpp"

class CharacterRotationSystem : public artemis::EntityProcessingSystem {
public:
	CharacterRotationSystem();
	~CharacterRotationSystem();

	void initialize() override;
	void processEntity(artemis::Entity& e) override;

private:
	artemis::ComponentMapper<CharacterRotationComponent> rotMapper;
	artemis::ComponentMapper<ControllerComponent> conMapper;
	artemis::ComponentMapper<Principia::TransformComponent> transMapper;
};