#include "characterController.h"
#include "../Application/applicationComponents.h"
#include "../scene.h"

using namespace Principia;
CharacterController::CharacterController()
{
	addComponentType<ControllerComponent>();
	addComponentType<CharacterComponent>();
}


CharacterController::~CharacterController()
{
}

void CharacterController::initialize()
{
	em = world->getEntityManager();
	sm = world->getSystemManager();


	inputMapper.init(*world);
	characterMapper.init(*world);
}

void CharacterController::processEntity(artemis::Entity & e)
{
	ControllerComponent* ic = inputMapper.get(e);
	CharacterComponent* pc = characterMapper.get(e);



	//GameComponent* gc = (GameComponent*)world->getSingleton()->getComponent<GameComponent>();
	//
	//float x;
	//gc->mode == GameMode::Traditional ? x = 0.f : x = ic->axis.x;
	TransformComponent* tc = (TransformComponent*)e.getComponent<TransformComponent>();
	//tc->world[3].x += ic->axis.x;
	//tc->world[3].z += ic->axis.y;
	glm::vec3 movement = glm::vec3(ic->axis.x, 0, ic->axis.y);
	movement *= pc->speed * 2.0f;
	//tc->global.position += movement;
	tc->local.position += movement * world->getDelta();

	if (ic->buttons[4].action > 0) {
		//if (ic->buttons[4].time > 0.2f) {
			fireProjectile(tc->local.position);
			ic->buttons[4].time = 0;
		//}
	}
}

void CharacterController::fireProjectile(const glm::vec3& pos)
{
	glm::vec3 sca = glm::vec3(0.4f);
	glm::vec3 dir = glm::vec3(0.f, 0.f, 1.f);
	float speed = 15.f;
	artemis::Entity* proj = SCENE.createGameShape("bp", pos, sca, 0, -1, true);
	NodeComponent* nc = (NodeComponent*)proj->getComponent<NodeComponent>();
	nc->isParent = true;
	proj->addComponent(new CollisionComponent(pos, sca.x));
	proj->addComponent(new GameObjectTypeComponent(GameObjectType::GAMEOBJECT_PLAYER));
	proj->addComponent(new ProjectileComponent(dir, speed));
	proj->refresh();
}

