#include "characterController.h"
#include "../Application/applicationComponents.h"

CharacterController::CharacterController()
{
	addComponentType<ControllerComponent>();
	addComponentType<CharacterComponent>();
}


CharacterController::~CharacterController()
{
}

void CharacterController::initialize()
{
	em = world->getEntityManager();
	sm = world->getSystemManager();


	inputMapper.init(*world);
	characterMapper.init(*world);
}

void CharacterController::processEntity(artemis::Entity & e)
{
	ControllerComponent* ic = inputMapper.get(e);
	CharacterComponent* pc = characterMapper.get(e);



	GameComponent* gc = (GameComponent*)world->getSingleton()->getComponent<GameComponent>();
	
	float x;
	gc->mode == GameMode::Traditional ? x = 0.f : x = ic->axis.x;

}

