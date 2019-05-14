/*
/* BallSpawnSystem Copyright (C) by Mike Murrell 2019
This system controls how the balls spawn
- First it checks to see if it has any balls to spawn
-	if true then checks the last time a ball has spawned
-		if enough time then it randomly spawns a ball + resettimer
-	if not enough time then checks if any balls in field
-		if no balls on field then spawn a ball + resettimer
-	iterate timer;
- iterate nothing
*/

#pragma once
#include <Artemis/Artemis.h>
#include "../scene.h"
#include "ballComponents.hpp"
#include "ballCollisionSystem.h"
#include "ballMovementSystem.h"


class BallSpawnSystem : public artemis::EntityProcessingSystem {
private:
	artemis::ComponentMapper<BallSpawnComponent> bsMapper;
	BallSpawnComponent* bs;
public:
	BallSpawnSystem();

	void initialize();
	void processEntity(artemis::Entity &e);

	void spawnBall();
	void removeBall();
};