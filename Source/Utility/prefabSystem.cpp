#include "prefabSystem.h"

Principia::PrefabSystem::PrefabSystem()
{
	addComponentType<PrefabComponent>();
	addComponentType<NodeComponent>();
}

Principia::PrefabSystem::~PrefabSystem()
{
}

void Principia::PrefabSystem::initialize()
{
	nodeMapper.init(*world);
	prefabMapper.init(*world);
}

void Principia::PrefabSystem::added(artemis::Entity & e)
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

void Principia::PrefabSystem::processEntity(artemis::Entity & e)
{
}

//tinyxml2::XMLError Principia::PrefabSystem::SavePrefab(std::string prefab, NodeComponent * node)
//{
//	XMLDocument doc;
//	XMLError eResult = doc.SaveFile((prefab).c_str());
//
//	XMLNode* pRoot = doc.NewElement("Root");
//	doc.InsertFirstChild(pRoot);
//
//	//XMLElement* element = 
//
//	return eResult;
//}
//
//tinyxml2::XMLError Principia::PrefabSystem::LoadPrefab(std::string prefab, NodeComponent * node)
//{
//	XMLDocument doc;
//	XMLError eResult = doc.LoadFile((prefab).c_str());
//
//	return eResult;
//}
