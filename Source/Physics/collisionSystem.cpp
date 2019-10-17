#include "collisionSystem.h"

Principium::CollisionSystem::CollisionSystem()
{
	addComponentType<CollisionComponent>();
	addComponentType<GameObjectTypeComponent>();
}

Principium::CollisionSystem::~CollisionSystem()
{
}

void Principium::CollisionSystem::initialize()
{
	colMapper.init(*world);
	gotMapper.init(*world);
}

void Principium::CollisionSystem::begin()
{
	for (auto enemy : enemies) {
		for (auto player : players) {
			checkCollision(*player, *enemy);
		}
	}
}

void Principium::CollisionSystem::end()
{
}

void Principium::CollisionSystem::added(artemis::Entity & e)
{
	GameObjectType t = gotMapper.get(e)->type;
	if (t & GameObjectType::GAMEOBJECT_ENEMY)
		enemies.insert(&e);
	else if (t & GameObjectType::GAMEOBJECT_PLAYER)
		players.insert(&e);
}

void Principium::CollisionSystem::removed(artemis::Entity & e)
{
	GameObjectType t = gotMapper.get(e)->type;
	if (t & GameObjectType::GAMEOBJECT_ENEMY)
		enemies.erase(&e);
	else if (t & GameObjectType::GAMEOBJECT_PLAYER)
		players.erase(&e);
}

void Principium::CollisionSystem::checkCollision(artemis::Entity & a, artemis::Entity & b)// const
{
	
	CollisionComponent* ccA = colMapper.get(a);
	CollisionComponent* ccB = colMapper.get(b);

	glm::vec3 diff = abs(ccA->position - ccB->position);
	glm::vec3 res = diff - ccA->radius;
	int comp = signbit(res.x) + signbit(res.y) + signbit(res.z);
	if (comp == 3) {
		ccB->collider = a.getId();
		ccB->state == CollisionState::Start ? ccB->state = CollisionState::Repeat : ccB->state = CollisionState::Start;

		ccA->collider = b.getId();
		ccA->state == CollisionState::Start ? ccA->state = CollisionState::Repeat : ccA->state = CollisionState::Start;

		std::cout << "Collision occured!1!!!!!!!!!";
	}
	
}
