#include "gameSystem.h"

GameSystem::GameSystem()
{
	addComponentType<GameComponent>();
	addComponentType<GlobalController>();
}

GameSystem::~GameSystem()
{
}

void GameSystem::initialize()
{
	gameMapper.init(*world);
	controlMapper.init(*world);

	game = gameMapper.get(*world->getSingleton());
	controller = controlMapper.get(*world->getSingleton());

	em = world->getEntityManager();
	sm = world->getSystemManager();
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();
	ts = (TransformSystem*)sm->getSystem<TransformSystem>();
	ui = (EngineUISystem*)sm->getSystem<EngineUISystem>();
	ps = (PhysicsSystem*)sm->getSystem<PhysicsSystem>();
	as = (AnimationSystem*)sm->getSystem<AnimationSystem>();

	cc = (CharacterController*)sm->getSystem<CharacterController>();// setSystem(new CharacterController());
	input = (ControllerSystem*)sm->getSystem<ControllerSystem>();// setSystem(new ControllerSystem());
	cc->initialize();
	input->initialize();

	//button = (ButtonSystem*)sm->getSystem<ButtonSystem>();

	cs = (CollisionSystem*)sm->setSystem(new CollisionSystem());
	spawner = (BallSpawnSystem*)sm->setSystem(new BallSpawnSystem());
	scorer = (BallScoreSystem*)sm->setSystem(new BallScoreSystem());
	bcs = (BallCollisionSystem*)sm->setSystem(new BallCollisionSystem());
	bms = (BallMovementSystem*)sm->setSystem(new BallMovementSystem());
	cs->initialize();
	bcs->initialize();
	bms->initialize();
	
	//set up singleton
	artemis::Entity* singleton = world->getSingleton();
	singleton->addComponent(new BallSpawnComponent());
	singleton->addComponent(new BallScoreComponent());
	singleton->refresh();

	spawner->change(*world->getSingleton());

	spawner->initialize();
	scorer->initialize();
}

void GameSystem::processEntity(artemis::Entity & e)
{
	updateInput();
	switch (game->state)
	{
	case GameState::Play:
	{

		ps->update();
		ps->process();

		cc->process();
		input->process();
		spawner->process();
		scorer->process();

		cs->process();
		bcs->process();
		bms->process();


		rs->mainLoop();
		break;
	}
	case GameState::Editor:
	{
		rs->mainLoop();
	}
	default:
		break;
	}
}

void GameSystem::updateInput() {
	glm::vec3 tempAxis = glm::vec3(0.f);
	for (int i = 0; i < NUM_GLOBAL_BUTTONS; ++i) {
		int action = INPUT.keys[controller->buttons[i].key];
		controller->buttons[i].action = action;
		if (action >= GLFW_PRESS) {
			controller->buttons[i].time += INPUT.deltaTime;
			if (i < 3)
				tempAxis[i] += 1.f;
			else if (i < 6)
				tempAxis[i - 3] -= 1.f;
			else if (i == 6) {
				togglePlayMode();
			}
		}
		else if (action == GLFW_RELEASE)
			controller->buttons[i].time = 0.f;
	}
	controller->axis = tempAxis;
}

void GameSystem::togglePlayMode()
{
	if (game->state == GameState::Editor) {
		game->state = GameState::Play;
		rs->togglePlayMode(true);
		ps->togglePlayMode();
	}
	else if (game->state == GameState::Play) {
		game->state = GameState::Editor;
		rs->togglePlayMode(false);
		ps->togglePlayMode();
	}
}



