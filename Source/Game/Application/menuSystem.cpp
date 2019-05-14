#include "menuSystem.h"
#include "../Rendering/renderSystem.h"
#include "../Utility/Input.h"
#include "controllerComponent.hpp"

MenuSystem::MenuSystem() {
	addComponentType<MenuComponent>();
	addComponentType<GUIComponent>();
	addComponentType<ControllerComponent>();
}

void MenuSystem::initialize() {
	menuMapper.init(*world);
	guiMapper.init(*world);
	controllerMapper.init(*world);
}

void MenuSystem::processEntity(artemis::Entity &e) {
	//timer or button ends it
	ControllerComponent* c = controllerMapper.get(e);
	for (int i = 0; i < 4; ++i) {
		if (c->buttons[i].action == 1) {
			e.removeComponent<MenuComponent>();
			e.addComponent(new GameComponent());
			e.refresh();
		}
	}
	if (c->buttons[9].action == 1) {
		glfwSetWindowShouldClose(WINDOW.getWindow(), 1);
	}
}

void MenuSystem::added(artemis::Entity &e) {
	GUIComponent* gui = guiMapper.get(e);
	gui->visible = true;
	gui->id = 2;
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
	rs->updateGui(gui);
}

void MenuSystem::removed(artemis::Entity &e) {
	GUIComponent* gui = guiMapper.get(e);
	gui->visible = false;
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
	rs->updateGui(gui);
}