#pragma once

#include "resourceManager.h"
#include "nodeComponent.hpp"
#include "Artemis/EntityManager.h"

namespace Principia {

#define SERIALIZENODE SerializeNode::get()

	class SerializeGameNode {
	public:
		virtual void LoadGameData(tinyxml2::XMLElement* node) {};
		virtual void SaveGameData(NodeComponent* node, tinyxml2::XMLDocument* doc) {};
	};

	//extern SerializeGameNode* sgn;
	class SerializeNode {
	private:
		SerializeNode() {};
		artemis::EntityManager* em;

	public:
		~SerializeNode() {};
		static SerializeNode& get() {
			static SerializeNode instance;
			return instance;
		}
		SerializeNode(SerializeNode const&) = delete;
		void operator=(SerializeNode const&) = delete;
		void SetEntityManager(artemis::EntityManager* e) { em = e; }
		tinyxml2::XMLElement* saveNode(NodeComponent* parent, tinyxml2::XMLDocument* doc);
		NodeComponent* loadNode(tinyxml2::XMLElement* node);
		std::vector<NodeComponent*> loadNodes(tinyxml2::XMLElement* start, tinyxml2::XMLElement* finish, NodeComponent* p);

	};
}