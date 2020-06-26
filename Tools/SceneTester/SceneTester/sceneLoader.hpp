#pragma once
#define GLM_FORCE_SSE2 // or GLM_FORCE_SSE42 if your processor supports it
#define GLM_FORCE_ALIGNED

#include <tinyxml2.h>
#include "transformComponent.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <string>


static const int COMPONENT_TRANSFORM = 0x02;


namespace SceneLoader {
	std::vector<TransformComponent*> loadNodes(tinyxml2::XMLElement* start, tinyxml2::XMLElement* finish, TransformComponent* par) {
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

			TransformComponent* trans = nullptr;

			if (flags & COMPONENT_TRANSFORM) {
				glm::vec3 pos;
				glm::vec3 rot;
				glm::vec3 sca;

				tinyxml2::XMLElement* transform = start->FirstChildElement("Transform");

				tinyxml2::XMLElement* position = transform->FirstChildElement("Position");
				position->QueryFloatAttribute("x", &pos.x);
				position->QueryFloatAttribute("y", &pos.y);
				position->QueryFloatAttribute("z", &pos.z);

				tinyxml2::XMLElement* rotation = transform->FirstChildElement("Rotation");
				rotation->QueryFloatAttribute("x", &rot.x);
				rotation->QueryFloatAttribute("y", &rot.y);
				rotation->QueryFloatAttribute("z", &rot.z);

				tinyxml2::XMLElement* scale = transform->FirstChildElement("Scale");
				scale->QueryFloatAttribute("x", &sca.x);
				scale->QueryFloatAttribute("y", &sca.y);
				scale->QueryFloatAttribute("z", &sca.z);

				trans = new TransformComponent(pos + par->local.position, rot * par->local.rotation, sca * par->local.scale);

				ret.push_back(trans);
			}

			////////////////// HAS CHILDREN ///////////////////
			if (hasChildren) {
				std::vector<TransformComponent*> children = loadNodes(start->FirstChildElement("Node"), start->LastChildElement("Node"), trans);
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
			TransformComponent* trans = nullptr;
			if (flags & COMPONENT_TRANSFORM) {
				glm::vec3 pos;
				glm::vec3 rot;
				glm::vec3 sca;

				tinyxml2::XMLElement* transform = start->FirstChildElement("Transform");

				tinyxml2::XMLElement* position = transform->FirstChildElement("Position");
				position->QueryFloatAttribute("x", &pos.x);
				position->QueryFloatAttribute("y", &pos.y);
				position->QueryFloatAttribute("z", &pos.z);

				tinyxml2::XMLElement* rotation = transform->FirstChildElement("Rotation");
				rotation->QueryFloatAttribute("x", &rot.x);
				rotation->QueryFloatAttribute("y", &rot.y);
				rotation->QueryFloatAttribute("z", &rot.z);

				tinyxml2::XMLElement* scale = transform->FirstChildElement("Scale");
				scale->QueryFloatAttribute("x", &sca.x);
				scale->QueryFloatAttribute("y", &sca.y);
				scale->QueryFloatAttribute("z", &sca.z);

				trans = new TransformComponent(pos, rot, sca);
				ret.push_back(trans);
			}

			////////////////// HAS CHILDREN ///////////////////
			if (hasChildren) {
				std::vector<TransformComponent*> children = loadNodes(start->FirstChildElement("Node"), start->LastChildElement("Node"), trans);
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
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError eResult = doc.LoadFile((dir + name + ".xml").c_str());
		tinyxml2::XMLNode * pNode = doc.FirstChild();

		tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Root");// ->FirstChildElement("Scene");
		tinyxml2::XMLElement* sceneN = pRoot->FirstChildElement("Scene");
		int sceneNumber;
		sceneN->QueryIntAttribute("Num", &sceneNumber);

		tinyxml2::XMLElement* first = pRoot->FirstChildElement("Node");
		tinyxml2::XMLElement* last = pRoot->LastChildElement("Node");

		std::vector<TransformComponent*> ret = loadNodes(first, last);
		std::vector<Bounds> Bounds;
		Bounds.reserve(ret.size());
		for (auto b : ret)
			Bounds.emplace_back(b->toBounds());
		return Bounds;
	}
}