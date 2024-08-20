#pragma once

#include "Artemis/EntityManager.h"
#include "../tinyxml2/tinyxml2.h"
#include <vector>
namespace Principia {

	struct NodeComponent;


#define SERIALIZENODE SerializeNode::get()

	class SerializeGameNode {
	public:
		virtual void LoadGameData(NodeComponent* node, tinyxml2::XMLElement* xNode) {};
		virtual void SaveGameData(NodeComponent* node, tinyxml2::XMLElement* xNode, tinyxml2::XMLDocument* doc) {};
	};

	//extern SerializeGameNode* sgn;
	class SerializeNode {
	private:
		SerializeNode() {};
		artemis::EntityManager* em = {};
		SerializeGameNode* gameData = {};

	public:
		~SerializeNode() {};
		static SerializeNode& get() {
			static SerializeNode instance;
			return instance;
		}
		SerializeNode(SerializeNode const&) = delete;
		void operator=(SerializeNode const&) = delete;
		inline void SetEntityManager(artemis::EntityManager* e) { em = e; }
		inline void SetGameData(SerializeGameNode* gd) { gameData = gd; }
		tinyxml2::XMLElement* saveNode(NodeComponent* parent, tinyxml2::XMLDocument* doc);
		NodeComponent* loadNode(tinyxml2::XMLElement* node);
		void loadNode(tinyxml2::XMLElement* node, artemis::Entity* e);
		std::vector<NodeComponent*> loadNodes(tinyxml2::XMLElement* start, NodeComponent* p);

	};
}
