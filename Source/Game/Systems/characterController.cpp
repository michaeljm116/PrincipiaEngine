#include "characterController.h"



CharacterController::CharacterController()
{
	addComponentType<ControllerComponent>();
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
	ControllerComponent* ic = inputMapper.get(e);
	AnimationComponent* ac = animationMapper.get(e);
	RigidBodyComponent* rb = rbMapper.get(e);
	CharacterComponent* pc = characterMapper.get(e);

	//btRigidBody* body = ps->getRigidBody(e);
	btCollisionObject* body = ps->getCollisionObject(e);
	//rb->direction = btVector3(ic->direction.x, ic->direction.y * pc->jumpSpeed, ic->direction.z) * pc->speed;
	//ps->applyForce(body, btVector3(-ic->direction.x, 0.f, -ic->direction.z) * pc->speed);
		ps->applyMovement(body, btVector3(ic->axis.x, 0.f, ic->axis.y) * 0.01 *  pc->speed);

	//if (state == CharacterState::GROUNDED) {
	//	state = CharacterState::JUMPED;
	//	ps->applyImpulse(body, btVector3(0.f, -ic->direction.y * pc->jumpSpeed, 0.f));
	//}
	//if (state == CharacterState::JUMPED) {
	//	if (ps->CheckGrounding(body))
	//		state = CharacterState::GROUNDED;
	//}

	if (ac != nullptr) {
		if (ic->buttons[7].time > 0) {
			ac->animIndex = 1;
		}
		else
			ac->animIndex = 0;
	}
}
