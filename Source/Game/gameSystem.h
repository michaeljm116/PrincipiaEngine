#pragma once
/* Game System Copyright (C) by Mike Murrell 
iirc this is rush-code from GGJ idk if i will
use or not
*/
#include "scene.h" 

#define GAMESYSTEM GameSystem::get()
class GameSystem {
private:
	GameSystem() {
		dummy.position = glm::vec3(0.f);
		dummy.rotation = glm::quat();
		dummy.scale = glm::vec3(1.f);
	};
public:
	~GameSystem() {};
	static GameSystem& get() {
		static GameSystem instance;
		return instance;
	}
	GameSystem(GameSystem const&) = delete;
	void operator=(GameSystem const&) = delete;

	void init(artemis::World& w);
	void update(float dt);
	void updateCamera();
	void start();
	//void processEntity(artemis::Entity& e);

private:
	artemis::World* world;
	artemis::EntityManager* em;
	artemis::SystemManager* sm;

	RenderSystem*	rs;
	TransformSystem* ts;
	PhysicsSystem* ps;
	EngineUISystem* ui;
	AnimationSystem* as;

	CharacterController* cc;
	InputSystem* input;
	ButtonSystem* button;

	sqt dummy;
	
	struct {
		TransformComponent* transform;
		CharacterState* state;
	} character;

};