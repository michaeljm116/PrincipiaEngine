#include "gameSceneSystem.h"
#include "../Utility/componentIncludes.h"
#include "../pch.h"
#include "../Physics/Components/collisionComponent.h"
#include "../Gameplay/Components/gameObjectTypeComponent.h"
#include "characterComponent.hpp"
#include "Components/enemyComponent.h"
#include "Components/chracterRotationComponent.h"
#include <unordered_set>

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
	//find player
	assignPlayer();

}

void GameSceneSystem::removed(artemis::Entity & e)
{
}

void GameSceneSystem::processEntity(artemis::Entity & e)
{
}

void GameSceneSystem::assignPlayer()
{
	NodeComponent* player = nullptr;
	std::unordered_set<std::string> us;
	us.insert("froku");
	for (auto& p : SCENE.parents) {
		if (us.find(p->name) != us.end()) {
			player = p;
			break;
		}
	}
	if (player == nullptr)
		return;
	artemis::Entity* pe = player->data;

	TransformComponent* t = (TransformComponent*)pe->getComponent<TransformComponent>();
	pe->addComponent(new CollisionComponent(glm::vec3(t->world[3]), 1));
	pe->addComponent(new CharacterComponent());
	pe->addComponent(new GameObjectTypeComponent(GameObjectType::GAMEOBJECT_PLAYER));
	pe->addComponent(new CharacterComponent());
	pe->addComponent(new CharacterRotationComponent(RotationDir::down));

	SCENE.insertController(player);
}

void GameSceneSystem::createEnemy(glm::vec3 pos, glm::vec3 rot, glm::vec3 sca, int matID)
{
	artemis::Entity* enemy = createModel("Low_poly_UFO", pos, rot, sca);
	enemy->addComponent(new CollisionComponent(pos, sca.x));
	enemy->addComponent(new GameObjectTypeComponent(GameObjectType::GAMEOBJECT_ENEMY));
	enemy->addComponent(new EnemyComponent());

	NodeComponent* enemyNode = (NodeComponent*)enemy->getComponent<NodeComponent>();
	int i = 0;
	for (auto child : enemyNode->children) {
		MaterialComponent* mc = (MaterialComponent*)child->data->getComponent<MaterialComponent>();
		i == 2 ? mc->matID = 2 : mc->matID = matID;
		++i;
	}
	enemy->refresh();
	
}

void GameSceneSystem::createPlayer(glm::vec3 pos, glm::vec3 rot, glm::vec3 sca)
{
	artemis::Entity* player = createModel("Low_poly_UFO", pos, rot, sca);
	player->addComponent(new CollisionComponent(pos, sca.x));
	player->addComponent(new GameObjectTypeComponent(GameObjectType::GAMEOBJECT_PLAYER));
	player->addComponent(new CharacterComponent());
	//player->addComponent(new ControllerComponent(1));
	NodeComponent* playerNode = (NodeComponent*)player->getComponent<NodeComponent>();
	SCENE.insertController(playerNode);
	
	int i = 0;
	for (auto child : playerNode->children) {
		MaterialComponent* mc = (MaterialComponent*)child->data->getComponent<MaterialComponent>();
		//PrimitiveComponent* pc = (PrimitiveComponent*)child->data->getComponent<PrimitiveComponent>();
		i == 2 ? mc->matID = 6 : mc->matID = 2;
		//i == 2 ? pc->matId =
		child->data->refresh();
		++i;
	}
	
	player->refresh();
}

artemis::Entity * GameSceneSystem::createModel(std::string, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca)
{
	rModel& resource = RESOURCEMANAGER.getModel("Low_poly_UFO");
	//Add Mesh Component and make it a parent node
	artemis::EntityManager* em = world->getEntityManager();
	artemis::Entity* entity = &em->create();
	TransformComponent* parentTransform = new TransformComponent(pos, rot, sca);
	NodeComponent* parent = new NodeComponent(entity, "enemy", COMPONENT_MODEL | COMPONENT_TRANSFORM | COMPONENT_AABB);// | COMPONENT_PRIMITIVE);

	parent->isParent = true;
	parent->isDynamic = true;
	entity->addComponent(parent);
	entity->addComponent(parentTransform);


	entity->refresh();

	//set up the subsetsx
	int i = 0;
	for (std::vector<rMesh>::const_iterator itr = resource.meshes.begin(); itr != resource.meshes.end(); itr++) {

		//Create Entity
		artemis::Entity* child = &em->create();

		//Set up subset data
		NodeComponent* childNode = new NodeComponent(child, parent);
		TransformComponent* childTransform = new TransformComponent(resource.meshes[i].center, resource.meshes[i].extents);

		child->addComponent(childNode);
		child->addComponent(childTransform);

		child->addComponent(new MeshComponent(resource.uniqueID, i));
		child->addComponent(new PrimitiveComponent(resource.uniqueID + i));
		child->addComponent(new MaterialComponent(0));
		child->addComponent(new RenderComponent(RenderType::RENDER_PRIMITIVE));
		//child->addComponent(new AABBComponent());	//SubsetAABB's point to the rendering system


		//childTransform->parentSM = &parentTransform->scaleM;
		//childTransform->parentRM = &parentTransform->rotationM;
		//childTransform->parentPM = &parentTransform->positionM;

		childNode->name = resource.meshes[i].name;// "Child " + std::to_string(i);
		childNode->flags |= COMPONENT_MESH | COMPONENT_MATERIAL | COMPONENT_AABB | COMPONENT_TRANSFORM | COMPONENT_PRIMITIVE;
		parent->children.push_back(childNode);
		//rs->addNode(childNode);

		++i;
		child->refresh();
	}
	for (i = 0; i < resource.shapes.size(); ++i) {
		//Create Entity
		artemis::Entity* child = &em->create();

		//Set up subset data
		NodeComponent* childNode = new NodeComponent(child, parent);
		TransformComponent* childTransform = new TransformComponent(resource.shapes[i].center, resource.shapes[i].extents);

		child->addComponent(childNode);
		child->addComponent(childTransform);
		child->addComponent(new PrimitiveComponent(resource.shapes[i].type));
		child->addComponent(new MaterialComponent(0));
		//child->addComponent(new AABBComponent()); //will this even be used???
		child->addComponent(new RenderComponent(RenderType::RENDER_PRIMITIVE));

		childNode->name = resource.shapes[i].name;
		childNode->flags |= COMPONENT_MATERIAL | COMPONENT_TRANSFORM | COMPONENT_PRIMITIVE;
		parent->children.push_back(childNode);
		//rs->addNode(childNode);

		child->refresh();
	}
	return entity;
}
