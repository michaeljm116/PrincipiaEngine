#pragma once
#include "Components/enemyComponent.h"

#include <Artemis/Entity.h>
#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>

#include "../transformComponent.hpp"
#include "../Utility/nodeComponent.hpp"
#include "../Physics/Components/collisionComponent.h"

#include <unordered_map>
class EnemyController : public artemis::EntityProcessingSystem {
public:

	EnemyController();
	~EnemyController();

	void initialize() override;

	//This will move the enemy,
	//if a bounds is hit it will flag a step & reverse
	void processEntity(artemis::Entity& e) override;

	//compares bounds
	void added(artemis::Entity& e) override;

	//destroys ship
	void removed(artemis::Entity& e) override;

	//check if a bounds has been hit 
	void begin() override;
	void end() override;
	
private:

	artemis::ComponentMapper<EnemyComponent> enemyMapper;
	artemis::ComponentMapper<Principia::CollisionComponent> colMapper;
	artemis::ComponentMapper<TransformComponent> transMapper;
	artemis::ComponentMapper<NodeComponent> nodeMapper;

	float* leftBounds;// = FLT_MAX;
	float* rightBounds;// = -FLT_MAX;

	int leftEnt;
	int rightEnt;
	std::vector<std::pair<glm::vec3*, int>> positions;
	std::unordered_map<int, int> pmap;
	float speed = 5.f;
	float hstep = 0.f;
	float vstep = -0.25f;
	float dir = 1.f;
	bool boundHit = false;
	float bhmul = 0.f;

	float stageLeftBounds = -40.f;
	float stageRightBounds = 40.f;

	bool heapify = false;
};
