#pragma once
#include "Components/gameSceneComponent.h"
#include "Components/gameObjectTypeComponent.h"

#include <Artemis/Artemis.h>

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
using namespace Principia;
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
	void assignPlayer();
	void createEnemy(glm::vec3 pos, glm::vec3 rot, glm::vec3 sca, int matID);
	void createPlayer(glm::vec3 pos, glm::vec3 rot, glm::vec3 sca);
	artemis::Entity* createModel(std::string, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca);
};