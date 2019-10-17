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

	//next create references to new systems
	game = (GameSystem*)sm->setSystem(new GameSystem());
	title = (TitleSystem*)sm->setSystem(new TitleSystem());
	menu = (MenuSystem*)sm->setSystem(new MenuSystem());
	pause = (PauseSystem*)sm->setSystem(new PauseSystem());

	//next initialize the old systems
	ui->initialize();
	controllers->initialize();

	//Next initialize the new systems;
	game->initialize();
	title->initialize();
	menu->initialize();
	pause->initialize();
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


	//render
	as->process();
	
	rs->process();// mainLoop();

	//build the bvh
	//bvh->rebuild = true;
	//bvh->build();
	//rs->updateBVH(bvh->prims, bvh->root, bvh->totalNodes);

	//get the input, if in editor use editor controller, else regular controllers
	if (ac->state == AppState::Editor)
		ui->process();
	else
		controllers->process();

}

void ApplicationSystem::instantGameStart()
{
	//WINDOW.toggleMaximized();
	WINDOW.toggleMaximized();
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
	rs->togglePlayMode(true);
	game->findGoals();
	//toggleEditor(AppState::Editor);
}



void ApplicationSystem::toggleEditor(AppState& s)
{
	//toggles the editor by passing or removing the editor component 
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

