#pragma once
#include "../../Utility/componentIncludes.h"
#include "characterComponent.hpp"
#include "../Application/controllerComponent.hpp"
#include "Components/projectileComponent.h"
#include "Components/chracterRotationComponent.h"

using namespace Principia;
class CharacterController : public artemis::EntityProcessingSystem
{
private:
	artemis::ComponentMapper<ControllerComponent> inputMapper;
	artemis::ComponentMapper<CharacterComponent> characterMapper;
	artemis::ComponentMapper<CharacterRotationComponent> rotMapper;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;

public:
	CharacterController();
	~CharacterController();

	void initialize();
	void processEntity(artemis::Entity& e);
	NodeComponent* characterNode;
	TransformComponent* characterTransform;
	struct {
		TransformComponent* transform;
		CameraComponent* component;
	} camera;

	bool reachedMax = false;

	void fireProjectile(const glm::vec3& pos);
	   
};

