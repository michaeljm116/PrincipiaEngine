#pragma once
#include <Artemis/EntityProcessingSystem.h>
#include <Artemis/ComponentMapper.h>
#include "nodeComponent.hpp"
#include "prefabComponent.h"
//#include "serialize-node.h"
//#include "resourceManager.h"

#include "../tinyxml2/tinyxml2.h"

namespace Principia {
	class PrefabSystem : public artemis::EntityProcessingSystem {
	public:
		PrefabSystem();
		~PrefabSystem(); 

		void initialize() override;
		void added(artemis::Entity &e) override;
		void processEntity(artemis::Entity &e) override;

	private:
		//artemis::ComponentMapper<NodeComponent> nodeMapper;
		artemis::ComponentMapper<PrefabComponent> prefabMapper = {};

		tinyxml2::XMLError SavePrefab(std::string prefab, NodeComponent* node);
		tinyxml2::XMLError LoadPrefab(std::string prefab, artemis::Entity* e);
	};
}