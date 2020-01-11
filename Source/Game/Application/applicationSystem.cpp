#include "applicationSystem.h"

ApplicationSystem::ApplicationSystem()
{
	addComponentType<ApplicationComponent>();
}

ApplicationSystem::~ApplicationSystem()
{
}

void ApplicationSystem::initialize()
{
	//Initialize the window
	glfwSetWindowUserPointer(WINDOW.getWindow(), this);
	glfwSetWindowSizeCallback(WINDOW.getWindow(), ApplicationSystem::onWindowResized);

	//Initialize the mappers
	appMapper.init(*world);

	//Get the managers
	artemis::EntityManager* em = world->getEntityManager();
	artemis::SystemManager* sm = world->getSystemManager();

	//First get references to the already started systems
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();
	as = (AnimationSystem*)sm->getSystem<AnimationSystem>();
	ui = (EngineUISystem*)sm->getSystem<EngineUISystem>();
	controllers = (ControllerSystem*)sm->getSystem<ControllerSystem>();// setSystem(new ControllerSystem());
	bvh = (BvhSystem*)sm->getSystem<BvhSystem>();

	//collision systems
	sysGrid = (GridSystem*)sm->setSystem(new GridSystem());
	col = (Principia::CollisionSystem*)sm->getSystem<Principia::CollisionSystem>();
	cws = (CollidedWithSystem*)sm->setSystem(new CollidedWithSystem());

	//next create references to new systems
	game = (GameSystem*)sm->setSystem(new GameSystem());
	title = (TitleSystem*)sm->setSystem(new TitleSystem());
	menu = (MenuSystem*)sm->setSystem(new MenuSystem());
	pause = (PauseSystem*)sm->setSystem(new PauseSystem());
	gss = (GameSceneSystem*)sm->setSystem(new GameSceneSystem());

	//next initialize the old systems
	ui->initialize();
	ui->init(&rs->getDeviceInfo());
	controllers->initialize();

	//Next initialize the new systems;
	game->initialize();
	title->initialize();
	menu->initialize();
	pause->initialize();
	gss->initialize();

	//col system initialize idk why colsys is in scene or whereever but ima do dis
	sysGrid->initialize();
	cws->initialize();
}

void ApplicationSystem::processEntity(artemis::Entity & e)
{
	ApplicationComponent* ac = appMapper.get(e);


	//handle transitions if one occurs
	//if (ac->transition) {
	//	ac->transition = false;
	//	switch (ac->state)
	//	{
	//	case AppState::TitleScreen:
	//		e.addComponent(new TitleComponent());
	//		break;
	//	case AppState::MainMenu:
	//		e.addComponent(new MenuComponent());
	//		break;
	//	case AppState::Play:
	//		e.addComponent(new GameComponent());
	//		break;
	//	case AppState::Paused:
	//		e.addComponent(new PauseComponent());
	//		break;
	//	case AppState::Editor:
	//		e.addComponent(new EditorComponent());
	//		break;
	//	default:
	//		break;
	//	}
	//	e.refresh();
	//}

	//run the systems
	game->process();
	title->process();
	menu->process();
	pause->process();

	//physics
	col->process();
	cws->process();

	//render
	as->process();

	//get the input, if in editor use editor controller, else regular controllers
	if (ac->state == AppState::Editor)
		ui->process();
	else
		controllers->process();

	bvh->build();
	rs->updateBVH(bvh->prims, bvh->root, bvh->totalNodes);
	rs->process();// mainLoop();

	uint32_t ii;
	rs->startFrame(ii);
	
	if(ui->visible)
		ui->showUI(&rs->submitInfo, ii);
	rs->endFrame(ii);

}

void ApplicationSystem::instantGameStart()
{
	//WINDOW.toggleMaximized();
	WINDOW.toggleMaximized();
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
	//rs->removeUI();
	//game->findGoals();
	AppState& as = appMapper.get(*world->getSingleton())->state;// = AppState::Editor;
	as = AppState::Play;
	world->getSingleton()->addComponent(new GameComponent());
	//toggleEditor(as);
}



void ApplicationSystem::toggleEditor(AppState& s)
{
	//toggles the editor by passing or removing the editor component 
	//Problem: this is too couplely and syou should be able to take off editor w/o gaming
	artemis::Entity* singleton = world->getSingleton();
	switch (s)
	{
	case AppState::Play:
		singleton->removeComponent<GameComponent>();
		singleton->addComponent(new EditorComponent());
		s = AppState::Editor;
		break;
	case AppState::Editor:
		singleton->removeComponent<EditorComponent>();
		singleton->addComponent(new GameComponent());
		s = AppState::Play;
		break;
	default:
		break;
	}
}
void ApplicationSystem::toggleAppState(AppState& s) {
	artemis::Entity* singleton = world->getSingleton();
	switch (s)
	{
	case AppState::TitleScreen:{
		singleton->removeComponent<TitleComponent>();
		singleton->addComponent(new MenuComponent());
		s = AppState::MainMenu;
		break;}
	case AppState::MainMenu:{
		singleton->removeComponent<MenuComponent>();
		singleton->addComponent(new GameComponent());
		s = AppState::Play;
		break;}
	default:
		break;
	}
}

void ApplicationSystem::togglePause(AppState& s)
{
	artemis::Entity* singleton = world->getSingleton();
	switch (s)
	{
	case AppState::Play:
		singleton->removeComponent<GameComponent>();
		singleton->addComponent(new PauseComponent());
		s = AppState::Paused;
		break;
	case AppState::Paused:
		singleton->removeComponent<PauseComponent>();
		singleton->addComponent(new GameComponent());
		s = AppState::Play;
		break;
	default:
		break;
	}
}

