#pragma once
#include "../../Utility/componentIncludes.h"
#include <Artemis/Artemis.h>
#include "../../Physics/physicsSystem.h"
enum class CharacterState {
	GROUNDED,
	JUMPED,
	INAIR
};
class CharacterController : public artemis::EntityProcessingSystem
{
private:
	artemis::ComponentMapper<ControllerComponent> inputMapper;
	artemis::ComponentMapper<RigidBodyComponent> rbMapper;
	artemis::ComponentMapper<AnimationComponent> animationMapper;
	artemis::ComponentMapper<CharacterComponent> characterMapper;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;
	PhysicsSystem* ps;

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

	CharacterState state = CharacterState::GROUNDED;


};

