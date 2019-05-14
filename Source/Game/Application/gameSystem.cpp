#include "gameSystem.h"
#include "../Rendering/renderSystem.h"

GameSystem::GameSystem()
{
	addComponentType<GameComponent>();
	addComponentType<ControllerComponent>();
}

GameSystem::~GameSystem()
{
}

void GameSystem::initialize()
{
	gameMapper.init(*world); 
	controllerMapper.init(*world);

	em = world->getEntityManager();
	sm = world->getSystemManager();
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();
	ts = (TransformSystem*)sm->getSystem<TransformSystem>();
	ui = (EngineUISystem*)sm->getSystem<EngineUISystem>();
	ps = (PhysicsSystem*)sm->getSystem<PhysicsSystem>();
	as = (AnimationSystem*)sm->getSystem<AnimationSystem>();

	cc = (CharacterController*)sm->getSystem<CharacterController>();// setSystem(new CharacterController());
	cc->initialize();

	//button = (ButtonSystem*)sm->getSystem<ButtonSystem>();

	cs = (CollisionSystem*)sm->setSystem(new CollisionSystem());
	spawner = (BallSpawnSystem*)sm->setSystem(new BallSpawnSystem());
	scorer = (BallScoreSystem*)sm->setSystem(new BallScoreSystem());
	bcs = (BallCollisionSystem*)sm->setSystem(new BallCollisionSystem());
	bms = (BallMovementSystem*)sm->setSystem(new BallMovementSystem());
	audio = (AudioSystem*)sm->setSystem(new AudioSystem());

	cs->initialize();
	bcs->initialize();
	bms->initialize();
	audio->initialize();

	//set up singleton
	artemis::Entity* singleton = world->getSingleton();
	singleton->addComponent(new BallSpawnComponent());
	singleton->refresh();

	spawner->change(*world->getSingleton());

	spawner->initialize();
	scorer->initialize();
	//findGoals();
}

void GameSystem::added(artemis::Entity & e)
{
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();

	for (int i = 0; i < goals.size(); ++i) {
		goals[i]->visible = true;
		rs->updateGuiNumber(goals[i]);
	}
}

void GameSystem::removed(artemis::Entity & e)
{
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();

	for (int i = 0; i < goals.size(); ++i) {
		goals[i]->visible = false;
		rs->updateGuiNumber(goals[i]);
	}

	e.addComponent(new PauseComponent());
	e.refresh();
}

void GameSystem::processEntity(artemis::Entity & e)
{
	ps->update();
	ps->process();

	cc->process();
	spawner->process();
	scorer->process();

	cs->process();
	bcs->process();
	bms->process();
	audio->process();

	ControllerComponent* c = controllerMapper.get(e);
	if (c->buttons[4].action == 1) {
		//c->buttons[4].action = 0;
		//c->buttons[4].time = 0.f;
		e.removeComponent<GameComponent>();
		e.refresh();
	}
}

void GameSystem::findGoals()
{
	int count = world->getEntityManager()->getEntityCount();
	NodeComponent* temp;
	for (int i = 0; i < count; ++i) {
		artemis::Entity &e =  world->getEntity(i);
		temp = (NodeComponent*)e.getComponent<NodeComponent>();
		if (temp != nullptr) {
			if (temp->tags == 8) {
				e.refresh();
				scorer->change(e);
				GUINumberComponent* g = (GUINumberComponent*)e.getComponent<GUINumberComponent>();
				goals.push_back(g);
			}
		}
	}
}




