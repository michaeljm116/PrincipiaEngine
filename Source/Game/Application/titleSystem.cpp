#include "titleSystem.h"

TitleSystem::TitleSystem() {
	addComponentType<TitleComponent>();
	addComponentType<GUIComponent>();
	addComponentType<GlobalController>();
}

void TitleSystem::initialize() {
	titleMapper.init(*world);
	guiMapper.init(*world);
	controllerMapper.init(*world);
}

void TitleSystem::processEntity(artemis::Entity &e) {
	//timer or button ends it
	TitleComponent* t = titleMapper.get(e);
	ControllerComponent* c = controllerMapper.get(e);
	for (int i = 0; i < 4; ++i) {
		if (c->buttons[i].action == 1) {
			e.removeComponent<TitleComponent>();
			e.refresh();
		}
	}
	t->timer -= world->getDelta();
	//if (t->timer < 0) {
	//	e.removeComponent<TitleComponent>();
	//	e.refresh();
	//}
	
}

void TitleSystem::added(artemis::Entity &e) {
	GUIComponent* gui = guiMapper.get(e);
	gui->visible = true;
	gui->id = 1;
	
}

void TitleSystem::removed(artemis::Entity &e) {
	GUIComponent* gui = guiMapper.get(e);
	gui->visible = false;
	e.addComponent(new MenuComponent());
	e.refresh();
}