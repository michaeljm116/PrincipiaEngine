#pragma once
/* Animation System Copyright (C) by Mike Murrell 
so this was before i moved everything to the GPU
there needs to be a complete rework in which i will
create bone bounding boxes
*/
#include "../Utility/componentIncludes.h"
#include "../pch.h"
#include "../Rendering/renderSystem.h"

class AnimationSystem : public artemis::EntityProcessingSystem
{
private:
	artemis::ComponentMapper<NodeComponent> nodeMapper;
	artemis::ComponentMapper<AnimationComponent> animationMapper;
	artemis::ComponentMapper<TransformComponent> transformMapper;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;
	RenderSystem* rs;
	float deltaTime;
	float globalTime = 0.f;

public:
	AnimationSystem();
	~AnimationSystem();

	void initialize();
	void processEntity(artemis::Entity& e);
	void addNode(NodeComponent* node);
	void update(float dt);



	glm::quat Interpolate(glm::quat start, glm::quat end, float delta);
	glm::mat4 BoneTransform(const Skeleton& skeleton, const rJointData& joints);
	glm::vec3 rotateAABB(const glm::quat & m, const glm::vec3 & extents);
private:

};

