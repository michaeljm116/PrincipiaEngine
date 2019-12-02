#include "characterRotationSystem.h"

CharacterRotationSystem::CharacterRotationSystem()
{
	addComponentType<CharacterRotationComponent>();
	addComponentType<ControllerComponent>();
	addComponentType<Principia::TransformComponent>();
}

CharacterRotationSystem::~CharacterRotationSystem()
{
}

void CharacterRotationSystem::initialize()
{
	rotMapper.init(*world);
	conMapper.init(*world);
	transMapper.init(*world);
}

void CharacterRotationSystem::processEntity(artemis::Entity & e)
{
	ControllerComponent* controller = conMapper.get(e);
	CharacterRotationComponent* rot = rotMapper.get(e);
	
	RotationDir d = rot->convert(controller->axis.x, controller->axis.y);
	if (d == RotationDir::default || d == rot->dir)
		return;
	else {
		float deg = 0;
		switch (d)
		{
		case RotationDir::downleft:
			deg = 225;
			rot->movementMul = .70710678f;
			break;
		case RotationDir::left:
			rot->movementMul = 1;
			deg = 270;
			break;
		case RotationDir::upleft:
			deg = 315;
			rot->movementMul = .70710678f;
			break;
		case RotationDir::down:
			rot->movementMul = 1;
			deg = 180;
			break;
		case RotationDir::up:
			rot->movementMul = 1;
			deg = 0;
			break;
		case RotationDir::downright:
			deg = 135;
			rot->movementMul = .70710678f;
			break;
		case RotationDir::right:
			rot->movementMul = 1;
			deg = 90;
			break;
		case RotationDir::upright:
			deg = 45;
			rot->movementMul = .70710678f;
			break;
		default:
			break;
		}

		Principia::TransformComponent* tc = transMapper.get(e);
		rot->dir = d;
		tc->eulerRotation.z = deg;
	}
}
