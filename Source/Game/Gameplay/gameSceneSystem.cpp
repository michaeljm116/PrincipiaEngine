#include "gameSceneSystem.h"
#include "../Utility/componentIncludes.h"
#include "../pch.h"
#include "../Physics/Components/collisionComponent.h"
#include "../Gameplay/Components/gameObjectTypeComponent.h"

GameSceneSystem::GameSceneSystem()
{
	addComponentType<GameSceneComponent>();
}

GameSceneSystem::~GameSceneSystem()
{
}

void GameSceneSystem::initialize()
{
	gscMapper.init(*world);
}

void GameSceneSystem::begin()
{
}

void GameSceneSystem::end()
{
}

void GameSceneSystem::added(artemis::Entity & e)
{
	int lvl = gscMapper.get(e)->levelIndex;
	e.removeComponent<GameSceneComponent>();
	//load scene
	glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f);
	artemis::Entity* enemy = SCENE.createGameShape("Enemy", pos, glm::vec3(1.f), 1, -1, true);
	enemy->addComponent(new Principia::CollisionComponent(pos, 1));
	enemy->addComponent(new Principia::GameObjectTypeComponent(Principia::GameObjectType::GAMEOBJECT_ENEMY));
	enemy->refresh();

	pos = glm::vec3(-2.f, 1.f, 0.f);
	artemis::Entity* player = SCENE.createGameShape("Player", pos, glm::vec3(1.f), 0, -1, true);
	player->addComponent(new Principia::CollisionComponent(pos, 1));
	player->addComponent(new Principia::GameObjectTypeComponent(Principia::GameObjectType::GAMEOBJECT_PLAYER));
	player->refresh();
}

void GameSceneSystem::removed(artemis::Entity & e)
{
}

void GameSceneSystem::processEntity(artemis::Entity & e)
{
}

void GameSceneSystem::createShape(std::string name, glm::vec3 pos, glm::vec3 scale, int matID, int type, bool dynamic)
{
	artemis::Entity& e = world->getEntityManager()->create();//&em->create();
	NodeComponent*		parent = new NodeComponent(&e, name, COMPONENT_MATERIAL | COMPONENT_TRANSFORM | COMPONENT_PRIMITIVE);
	TransformComponent* trans = new TransformComponent(pos, glm::vec3(0.f), scale);

	e.addComponent(new PrimitiveComponent(type));
	e.addComponent(new MaterialComponent(matID));
	e.addComponent(new RenderComponent(RenderType::RENDER_PRIMITIVE));
	e.addComponent(trans);
	e.addComponent(parent);

	parent->isDynamic = dynamic;
	e.refresh();
	//ts->recursiveTransform(parent);
	//rs->updateObjectMemory();

}
