#pragma once
/* Transform system Copyright (C) by Mike Murrell 
this system currently only transforms certain objects
if its static it doesn't transform at all which makes 
me wonder if i should have a transform system vs 
a movement system.
*/
//#include "../Utility/componentIncludes.h"

#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>
#include <glm/mat3x3.hpp>



namespace Principia {

	struct TransformComponent;
	struct NodeComponent;
	class RenderSystem;

	class TransformSystem : public artemis::EntityProcessingSystem
	{
	private:

		artemis::ComponentMapper<TransformComponent> transformMapper = {};
		artemis::ComponentMapper<NodeComponent> nodeMapper = {};
		RenderSystem* rs = {};


	public:
		TransformSystem();
		~TransformSystem();
		void initialize();
		void added(artemis::Entity &e) override;

		void processEntity(artemis::Entity &e);
		void SQTTransform(NodeComponent* nc);// glm::vec3 sca, glm::quat rot, glm::vec3 pos);
		void recursiveTransform(NodeComponent* nc);// , TransformComponent global = TransformComponent());
		glm::vec3 rotateAABB(const glm::mat3& m);//, const glm::vec3& extents);
		void geometryTransformConverter(NodeComponent* nc);
	};

}