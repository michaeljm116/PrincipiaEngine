#include "characterController.h"
#include "../gameComponent.hpp"


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

	ps = (PhysicsSystem*)sm->getSystem<PhysicsSystem>();

	inputMapper.init(*world);
	characterMapper.init(*world);
}

void CharacterController::processEntity(artemis::Entity & e)
{
	ControllerComponent* ic = inputMapper.get(e);
	CharacterComponent* pc = characterMapper.get(e);


	btCollisionObject* body = ps->getCollisionObject(e);

	GameComponent* gc = (GameComponent*)world->getSingleton()->getComponent<GameComponent>();
	
	float x;
	gc->mode == GameMode::Traditional ? x = 0.f : x = ic->axis.x;
	ps->applyMovement(body, btVector3(x, 0.f, ic->axis.y) * 0.01 *  pc->speed);

}

