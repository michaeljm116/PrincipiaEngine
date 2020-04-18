#pragma once
#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>
#include "../Utility/nodeComponent.hpp"
#include "../Utility/serialize-node.h"
#include "animationComponent.hpp"
//#include "resourceManager.h"

namespace Principia {
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