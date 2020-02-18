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
		addComponentType<NodeComponent>();
	}

	PrefabSystem::~PrefabSystem()
	{
	}

	void PrefabSystem::initialize()
	{
		nodeMapper.init(*world);
		prefabMapper.init(*world);
	}

	void PrefabSystem::added(artemis::Entity & e)
	{
		PrefabComponent* pc = prefabMapper.get(e);
		NodeComponent* nc = nodeMapper.get(e);
		if (pc->save)
		{

		}
		else
		{

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

	tinyxml2::XMLError PrefabSystem::LoadPrefab(std::string prefab, NodeComponent * node)
	{
		XMLDocument doc;
		XMLError eResult = doc.LoadFile((prefab).c_str());

		return eResult;
	}
}