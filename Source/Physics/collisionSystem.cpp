#include "collisionSystem.h"
#include "../Game/transformComponent.hpp"

Principia::CollisionSystem::CollisionSystem()
{
	addComponentType<CollisionComponent>();
	addComponentType<GameObjectTypeComponent>();
}

Principia::CollisionSystem::~CollisionSystem()
{
}

void Principia::CollisionSystem::initialize()
{
	colMapper.init(*world);
	gotMapper.init(*world);
}

void Principia::CollisionSystem::begin()
{
	for (auto enemy : enemies) {
		for (auto player : players) {
			checkCollision(*player, *enemy);
		}
	}
}

void Principia::CollisionSystem::end()
{
}

void Principia::CollisionSystem::added(artemis::Entity & e)
{
	GameObjectType t = gotMapper.get(e)->type;
	if (t & GameObjectType::GAMEOBJECT_ENEMY)
		enemies.insert(&e);
	else if (t & GameObjectType::GAMEOBJECT_PLAYER)
		players.insert(&e);
}

void Principia::CollisionSystem::removed(artemis::Entity & e)
{
	GameObjectType t = gotMapper.get(e)->type;
	if (t & GameObjectType::GAMEOBJECT_ENEMY)
		enemies.erase(&e);
	else if (t & GameObjectType::GAMEOBJECT_PLAYER)
		players.erase(&e);
}

void Principia::CollisionSystem::processEntity(artemis::Entity & e)
{
	TransformComponent* tc = (TransformComponent*) e.getComponent<TransformComponent>();
	colMapper.get(e)->position = tc->world[3];// tc->global.position;
}

void Principia::CollisionSystem::checkCollision(artemis::Entity & a, artemis::Entity & b)// const
{
	
	CollisionComponent* ccA = colMapper.get(a);
	CollisionComponent* ccB = colMapper.get(b);

	float dist = sqrt(pow(ccA->position.x - ccB->position.x, 2) + pow(ccA->position.y - ccB->position.y, 2) + pow(ccA->position.z - ccB->position.z, 2));
	if(dist < ccA->radius + ccB->radius)
/*	
	glm::vec3 diff = abs(ccA->position - ccB->position);
	glm::vec3 res = diff - ccA->radius;
	int comp = signbit(res.x) + signbit(res.y) + signbit(res.z);
	if (comp == 3) */{
		ccB->collider = a.getId();
		ccB->state == CollisionState::Start ? ccB->state = CollisionState::Repeat : ccB->state = CollisionState::Start;

		ccA->collider = b.getId();
		ccA->state == CollisionState::Start ? ccA->state = CollisionState::Repeat : ccA->state = CollisionState::Start;

		std::cout << "Collision occured!1!!!!!!!!!";
	}
	
}
