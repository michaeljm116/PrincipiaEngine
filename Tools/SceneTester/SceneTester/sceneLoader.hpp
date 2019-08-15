#pragma once
#include "tinyxml2.h"
#include "transformComponent.hpp"
#include <glm.hpp>
#include <vector>


static const int COMPONENT_TRANSFORM = 0x02;

using namespace tinyxml2;
std::vector<TransformComponent*> loadNodes(tinyxml2::XMLElement* start, tinyxml2::XMLElement* finish) {
	std::vector<TransformComponent*> ret;
	bool lastOne = false;
	// node
	while (!lastOne) {

		const char* name;
		bool hasChildren;
		int flags;
		int tags;
		bool dynamic;

		start->QueryStringAttribute("Name", &name);
		start->QueryBoolAttribute("hasChildren", &hasChildren);
		start->QueryIntAttribute("Flags", &flags);
		start->QueryIntAttribute("Tags", &tags);
		start->QueryBoolAttribute("Dynamic", &dynamic);

		if (flags & COMPONENT_TRANSFORM) {
			glm::vec3 pos;
			glm::vec3 rot;
			glm::vec3 sca;

			XMLElement* transform = start->FirstChildElement("Transform");

			XMLElement* position = transform->FirstChildElement("Position");
			position->QueryFloatAttribute("x", &pos.x);
			position->QueryFloatAttribute("y", &pos.y);
			position->QueryFloatAttribute("z", &pos.z);

			XMLElement* rotation = transform->FirstChildElement("Rotation");
			rotation->QueryFloatAttribute("x", &rot.x);
			rotation->QueryFloatAttribute("y", &rot.y);
			rotation->QueryFloatAttribute("z", &rot.z);

			XMLElement* scale = transform->FirstChildElement("Scale");
			scale->QueryFloatAttribute("x", &sca.x);
			scale->QueryFloatAttribute("y", &sca.y);
			scale->QueryFloatAttribute("z", &sca.z);

			TransformComponent* trans = new TransformComponent(pos, rot, sca);

			ret.push_back(trans);
		}

		////////////////// HAS CHILDREN ///////////////////
		if (hasChildren) {
			std::vector<TransformComponent*> children = loadNodes(start->FirstChildElement("Node"), start->LastChildElement("Node"));
			for (auto c : children)
				ret.push_back(c);
		}
		///////////////////////////////////////////////////

		//loop it up;
		if (start != finish)
			start = start->NextSiblingElement();
		else
			lastOne = true;
	}
	return ret;
}

std::vector<Bounds> LoadScene(std::string dir, std::string name)
{
	XMLDocument doc;
	XMLError eResult = doc.LoadFile((dir + name + ".xml").c_str());
	XMLNode * pNode = doc.FirstChild();

	XMLElement* pRoot = doc.FirstChildElement("Root");// ->FirstChildElement("Scene");
	XMLElement* sceneN = pRoot->FirstChildElement("Scene");
	int sceneNumber;
	sceneN->QueryIntAttribute("Num", &sceneNumber);

	XMLElement* first = pRoot->FirstChildElement("Node");
	XMLElement* last = pRoot->LastChildElement("Node");

	std::vector<TransformComponent*> ret = loadNodes(first, last);
	std::vector<Bounds> bounds;
	bounds.reserve(ret.size());
	for (auto b : ret)
		bounds.emplace_back(b->toBounds());
	return bounds;
}