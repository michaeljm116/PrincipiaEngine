#pragma once
/* Transform system Copyright (C) by Mike Murrell 
this system currently only transforms certain objects
if its static it doesn't transform at all which makes 
me wonder if i should have a transform system vs 
a movement system.
*/
#include "../Utility/componentIncludes.h"
#include "../Rendering/renderSystem.h"
#include "../Physics/Components/collisionComponent.h"

class TransformSystem : public artemis::EntityProcessingSystem
{
private:
	//glm::mat4 world;

	artemis::ComponentMapper<TransformComponent> transformMapper;
	artemis::ComponentMapper<NodeComponent> nodeMapper;
	//artemis::ComponentMapper<Principia::CollisionComponent> colMapper;

	artemis::EntityManager* em;
	artemis::SystemManager* sm;
	RenderSystem* rs;
	

public:
	TransformSystem();
	~TransformSystem();
	void initialize();
	void added(artemis::Entity &e) override;

	//void Scale(NodeComponent* nc);
	//void Translate(NodeComponent* nc, glm::mat4 pt);

	void processEntity(artemis::Entity &e);
	void SQTTransform(NodeComponent* nc, sqt parent);// glm::vec3 sca, glm::quat rot, glm::vec3 pos);
	void regularTransform(NodeComponent* nc, TransformComponent* parent);
	void recursiveTransform(NodeComponent* nc);// , TransformComponent global = TransformComponent());
	glm::vec3 rotateAABB(const glm::quat& m, const glm::vec3& extents);
	void geometryTransformConverter(NodeComponent* nc);
};
