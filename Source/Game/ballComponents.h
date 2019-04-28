#pragma once
#include <unordered_map>
#include "gameComponent.hpp"

enum class HitType {
	None, Wall, Player, Goal
};

struct BallComponent : artemis::Component {
	HitType hitType;
	glm::vec3 direction;
	float speed;
	float timeSinceHit = 0; //makes sure there's not multi collisions
	int goalHit = 0;
	BallComponent() { hitType = HitType::None; 
	int rando = rand() % 4;
	float x = 0.5f;  float y = 0.5f;

	switch (rando) {
	case 0:
		x = -.5f;
		break;
	case 1:
		y = -.5f;
		break;
	case 2: 
		x = -.5f;
		y = -.5f;
		break;
	}

	direction = glm::vec3(x, 0.f, y), speed = 20.f; }
};

enum class BallSpawnState {
	Ready, Waiting, Remove
};
struct BallSpawnComponent : artemis::Component {
	int maxballs = 1;
	int activeBalls = 0;
	int removedBallID = 0;
	float timer = 0.f;
	float spawnPoint = 0.f;
	float maxTime = 2.f;
	BallSpawnState state = BallSpawnState::Ready;
	BallSpawnComponent() {};
};

struct BallScoreComponent : artemis::Component {
	int scores[2];
	BallScoreComponent() { scores[0] = 0; scores[1] = 0; };
};
