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
	as = (AnimationSystem*)sm->getSystem<AnimationSystem>();

	cc = (CharacterController*)sm->getSystem<CharacterController>();// setSystem(new CharacterController());
	cc->initialize();
	ec = (EnemyController*)sm->setSystem(new EnemyController());
	ec->initialize();
	pms = (ProjectileMovementSystem*)sm->setSystem(new ProjectileMovementSystem());
	pms->initialize();
	crs = (CharacterRotationSystem*)sm->setSystem(new CharacterRotationSystem());
	crs->initialize();
	//button = (ButtonSystem*)sm->getSystem<ButtonSystem>();

	//audio = (AudioSystem*)sm->setSystem(new AudioSystem());

	//audio->initialize();


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
}

void GameSystem::processEntity(artemis::Entity & e)
{
	pms->process();
	cc->process();
	//ec->process();
	crs->process();
	ts->process();
	//audio->process();

	ControllerComponent* c = controllerMapper.get(e);

	//pause button
	if (c->buttons[9].action == 1) {
		//c->buttons[4].action = 0;
		//c->buttons[4].time = 0.f;
		RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();

		for (int i = 0; i < goals.size(); ++i) {
			goals[i]->visible = false;
			rs->updateGuiNumber(goals[i]);
		}

		e.removeComponent<GameComponent>();
		e.addComponent(new PauseComponent());
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
				//scorer->change(e);
				GUINumberComponent* g = (GUINumberComponent*)e.getComponent<GUINumberComponent>();
				goals.push_back(g);
			}
		}
	}
}




