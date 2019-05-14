#include "pauseSystem.h"
#include "../Rendering/renderSystem.h"

PauseSystem::PauseSystem() {
	addComponentType<PauseComponent>();
	addComponentType<GUIComponent>();
	addComponentType<ControllerComponent>();
}

void PauseSystem::initialize() {
	pauseMapper.init(*world);
	guiMapper.init(*world);
	controllerMapper.init(*world);
}

void PauseSystem::processEntity(artemis::Entity &e) {
	//pause button ends it
	ControllerComponent* c = controllerMapper.get(e);
	if (c->buttons[4].action == 1) {
		//c->buttons[4].action = 0;
		//c->buttons[4].time = 0.f;
		e.removeComponent<PauseComponent>();
		e.addComponent(new GameComponent());
		e.refresh();
	}
	if (c->buttons[9].action == 1) {
		//glfwSetWindowShouldClose(WINDOW.getWindow(), 1);
		e.removeComponent<PauseComponent>();
		e.addComponent(new MenuComponent());
		e.refresh();
	}
}

void PauseSystem::added(artemis::Entity &e) {
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();

	GUIComponent* gui = guiMapper.get(e);
	gui->visible = true;
	gui->id = 3;

	rs->updateGui(gui);
}

void PauseSystem::removed(artemis::Entity &e) {
	//RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
	//GUIComponent* gui = guiMapper.get(e);
	//gui->visible = false;
	//rs->updateGui(gui);
	//e.addComponent(new GameComponent());
	//e.refresh();
}