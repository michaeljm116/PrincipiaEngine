#include "../pch.h"
#include "prefabSystem.h"
#include "serialize-node.h"

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != tinyxml2::XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

using namespace tinyxml2;


namespace Principia {
	PrefabSystem::PrefabSystem()
	{
		addComponentType<PrefabComponent>();
	}

	PrefabSystem::~PrefabSystem()
	{
	}

	void PrefabSystem::initialize()
	{
		prefabMapper.init(*world);
	}

	void PrefabSystem::added(artemis::Entity & e)
	{
		PrefabComponent* pc = prefabMapper.get(e);
		tinyxml2::XMLError eR;
		if (pc->save)
		{
			NodeComponent* nc = (NodeComponent*)e.getComponent<NodeComponent>();
			assert(nc != nullptr);
			eR = SavePrefab(pc->dir + pc->name, nc);
		}
		else if(pc->load_needed)
		{
			eR = LoadPrefab(pc->dir + pc->name, &e);
		}
	}

	void PrefabSystem::processEntity(artemis::Entity & e)
	{
	}

	tinyxml2::XMLError PrefabSystem::SavePrefab(std::string prefab, NodeComponent * node)
	{
		//Create the XML Document
		XMLDocument doc;

		//Create root of the tree
		XMLNode* pRoot = doc.NewElement("Root");
		doc.InsertFirstChild(pRoot);

		//Insert all the relevant data
		XMLElement * element = SERIALIZENODE.saveNode(node, &doc);
		pRoot->InsertEndChild(element);

		//Save the file and check for any errors
		XMLError eResult = doc.SaveFile((prefab + ".prefab").c_str());
		XMLCheckResult(eResult);

		//return that sumbish
		return eResult;
	}

	tinyxml2::XMLError PrefabSystem::LoadPrefab(std::string prefab, artemis::Entity* e)
	{
		XMLDocument doc;
		XMLError eResult = doc.LoadFile((prefab + ".prefab").c_str());
		XMLCheckResult(eResult);
		XMLNode * pNode = doc.FirstChild();
		XMLElement* pRoot = doc.FirstChildElement("Root");

		SERIALIZENODE.loadNode(pRoot->FirstChildElement("Node"), e);

		return eResult;
	}
}