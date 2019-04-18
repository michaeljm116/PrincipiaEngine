#pragma once
#include "../../Physics/physicsSystem.h"

class ButtonSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<ControllerComponent> inputMapper;
	artemis::ComponentMapper<SwitchComponent> buttonMapper;
	artemis::ComponentMapper<NodeComponent> nodeMapper;
	artemis::ComponentMapper<MaterialComponent> materialMapper;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;
	RenderSystem* rs;
	TransformSystem* ts;

public:
	ButtonSystem();
	~ButtonSystem();

	void initialize();
	void processEntity(artemis::Entity& e);
};