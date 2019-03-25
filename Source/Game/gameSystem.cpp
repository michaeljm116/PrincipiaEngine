#include "gameSystem.h"

void GameSystem::init(artemis::World & w)
{
	world = &w;
	em = world->getEntityManager();
	sm = world->getSystemManager();
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();
	ts = (TransformSystem*)sm->getSystem<TransformSystem>();
	ui = (EngineUISystem*)sm->getSystem<EngineUISystem>();
	ps = (PhysicsSystem*)sm->getSystem<PhysicsSystem>();
	as = (AnimationSystem*)sm->getSystem<AnimationSystem>();

	cc = (CharacterController*)sm->getSystem<CharacterController>();// setSystem(new CharacterController());
	input = (InputSystem*)sm->getSystem<InputSystem>();// setSystem(new InputSystem());
	button = (ButtonSystem*)sm->getSystem<ButtonSystem>();

	cc->initialize();
	input->initialize();
	button->initialize();
}

void GameSystem::update(float dt)
{
	input->process();
	cc->process();
	button->process();
	//updateLight(dt);
}

void GameSystem::updateCamera() {
	//cc->cameraTransform->global.position = cc->characterTransform->global.position;
	//cc->cameraTransform->global.position.y += 1.f;
	cc->camera.component->pos = cc->characterTransform->global.position;
	cc->camera.component->pos.y += 35.f;
	rs->updateCamera(cc->camera.component);
}
