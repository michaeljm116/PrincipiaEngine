#pragma once
#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>
//#include "resourceManager.h"

namespace Principia {
	struct PoseComponent;

	class PoseSystem : public artemis::EntityProcessingSystem {
	public:
		PoseSystem();
		~PoseSystem();

		void initialize() override;
		void added(artemis::Entity &e) override;
		void processEntity(artemis::Entity &e) override;

	private:
		//artemis::ComponentMapper<NodeComponent> nodeMapper;
		artemis::ComponentMapper<PoseComponent> poseMapper;
	};
}