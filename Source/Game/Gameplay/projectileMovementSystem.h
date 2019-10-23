#pragma once
#include "Components/projectileComponent.h"
#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/Entity.h>
#include <Artemis/ComponentMapper.h>
#include "../Physics/Components/collisionComponent.h"
#include "../Game/transformComponent.hpp"

//using namespace Principia;
//moves the projectiles in a particular direction and yeah thats about it
class ProjectileMovementSystem : public artemis::EntityProcessingSystem {
public:
	ProjectileMovementSystem();
	~ProjectileMovementSystem();

	void initialize() override;
	void removed(artemis::Entity& e) override;
	void processEntity(artemis::Entity& e) override;

private:
	artemis::ComponentMapper<ProjectileComponent> projMapper;
	artemis::ComponentMapper<TransformComponent> transMapper;
	artemis::ComponentMapper<Principia::CollisionComponent> colMapper;
};

