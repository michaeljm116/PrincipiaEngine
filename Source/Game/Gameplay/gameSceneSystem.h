#pragma once
#include "Components/gameSceneComponent.h"
#include "Components/gameObjectTypeComponent.h"

#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>
#include <Artemis/Entity.h>

#include <glm/glm.hpp>
#include "../scene.h"

/*
Eventually: it will allow you to load and save your own game scenes
Now: it will just pop things up for a test run so its a scene start spawning system
When you add a game scene component it will start and then pop that component out
- so that its not constantly processing

It creates entities of:
 - Player
 - Enemies

*/
class GameSceneSystem : public artemis::EntityProcessingSystem {

public:
	GameSceneSystem();
	~GameSceneSystem();

	void initialize() override;
	void begin() override;
	void end() override;
	void added(artemis::Entity &e) override;
	void removed(artemis::Entity &e) override;
	void processEntity(artemis::Entity &e) override;


private:
	artemis::ComponentMapper<GameSceneComponent> gscMapper;
	void createShape(std::string name, glm::vec3 pos, glm::vec3 scale, int matID, int type, bool dynamic);
};