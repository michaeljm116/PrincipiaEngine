#include "characterController.h"



CharacterController::CharacterController()
{
	addComponentType<InputComponent>();
	addComponentType<RigidBodyComponent>();
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

	animationMapper.init(*world);
	inputMapper.init(*world);
	rbMapper.init(*world);
	characterMapper.init(*world);
}

void CharacterController::processEntity(artemis::Entity & e)
{
	InputComponent* ic = inputMapper.get(e);
	AnimationComponent* ac = animationMapper.get(e);
	RigidBodyComponent* rb = rbMapper.get(e);
	CharacterComponent* pc = characterMapper.get(e);

	btRigidBody* body = ps->getRigidBody(e);
	//rb->direction = btVector3(ic->direction.x, ic->direction.y * pc->jumpSpeed, ic->direction.z) * pc->speed;
	ps->applyForce(body, btVector3(-ic->direction.x, 0.f, -ic->direction.z) * pc->speed);
	if (state == CharacterState::GROUNDED) {
		state = CharacterState::JUMPED;
		ps->applyImpulse(body, btVector3(0.f, -ic->direction.y * pc->jumpSpeed, 0.f));
	}
	if (state == CharacterState::JUMPED) {
		if (ps->CheckGrounding(body))
			state = CharacterState::GROUNDED;
	}

	if (ac != nullptr) {
		if (ic->button[1] > 0) {
			ac->animIndex = 1;
		}
		else
			ac->animIndex = 0;
	}
}
