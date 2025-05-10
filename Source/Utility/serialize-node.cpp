#include "../pch.h"
#include "serialize-node.h"
#include "../Physics/Components/collisionComponent.h"
#include "../Physics/Components/dynamicComponent.h"
#include "../Physics/Components/staticComponent.h"
#include "prefabComponent.h"
#include "resourceManager.h"
#include "componentIncludes.h"

#pragma once
using namespace tinyxml2;
namespace Principia {

	tinyxml2::XMLElement * SerializeNode::saveNode(NodeComponent * parent, tinyxml2::XMLDocument * doc)
	{
		
		//Save name
		XMLElement * pNode = doc->NewElement("Node");
		pNode->SetAttribute("Name", parent->name.c_str());

		//find out if has children
		bool hasChildren = parent->children.size() > 0 ? true : false;
		pNode->SetAttribute("hasChildren", hasChildren);

		//Save node flags
		pNode->SetAttribute("eFlags", parent->engineFlags);

		//Save node tags
		pNode->SetAttribute("gFlags", parent->gameFlags);

		//Save dynamicness
		pNode->SetAttribute("Dynamic", parent->isDynamic);

		//save component infos
		//TransformInfo
		if (parent->engineFlags & COMPONENT_TRANSFORM) {
			TransformComponent* t = (TransformComponent*)parent->data->getComponent<TransformComponent>();
			XMLElement* pTransform = doc->NewElement("Transform");
			XMLElement* pPosition = doc->NewElement("Position");
			pPosition->SetAttribute("x", t->local.position.x);
			pPosition->SetAttribute("y", t->local.position.y);
			pPosition->SetAttribute("z", t->local.position.z);

			XMLElement* pRotation = doc->NewElement("Rotation");
			pRotation->SetAttribute("x", t->eulerRotation.x);
			pRotation->SetAttribute("y", t->eulerRotation.y);
			pRotation->SetAttribute("z", t->eulerRotation.z);

			XMLElement* pScale = doc->NewElement("Scale");
			pScale->SetAttribute("x", t->local.scale.x);
			pScale->SetAttribute("y", t->local.scale.y);
			pScale->SetAttribute("z", t->local.scale.z);

			pTransform->InsertFirstChild(pPosition);
			pTransform->InsertAfterChild(pPosition, pRotation);
			pTransform->InsertEndChild(pScale);

			pNode->InsertEndChild(pTransform);
		}
		if (parent->engineFlags & COMPONENT_PREFAB) {
			auto* prefab_component = (PrefabComponent*)parent->data->getComponent<PrefabComponent>();
			//If you don't want this prefab serialized, make sure it can't be serialized
			if (!prefab_component->can_serialize) {
				parent->engineFlags = COMPONENT_TRANSFORM | COMPONENT_PREFAB;
			}
			XMLElement* prefab_element = doc->NewElement("Prefab");
			XMLElement* prefab_file = doc->NewElement("FilePath");
			XMLElement* prefab_options = doc->NewElement("PrefabOptions");

			prefab_file->SetAttribute("name", prefab_component->name.c_str());
			prefab_file->SetAttribute("dir", prefab_component->dir.c_str());
			prefab_options->SetAttribute("save", prefab_component->save);
			prefab_options->SetAttribute("load", prefab_component->load_needed);
			prefab_options->SetAttribute("serialize", prefab_component->can_serialize);

			prefab_element->InsertFirstChild(prefab_file);
			prefab_element->InsertEndChild(prefab_options);
			
			pNode->InsertEndChild(prefab_element);
		}
		if (parent->engineFlags & COMPONENT_GUI) {
			GUIComponent* gui;
			parent->gameFlags == 8 ?
				gui = (GUINumberComponent*)parent->data->getComponent<GUINumberComponent>() :
				gui = (GUIComponent*)parent->data->getComponent<GUIComponent>();
			XMLElement* pGUI = doc->NewElement("GUI");
			XMLElement* pPos = doc->NewElement("Position");
			pPos->SetAttribute("x", gui->alignMin.x);
			pPos->SetAttribute("y", gui->alignMin.y);

			XMLElement* pExt = doc->NewElement("Extent");
			pExt->SetAttribute("x", gui->extents.x);
			pExt->SetAttribute("y", gui->extents.y);

			XMLElement* pAlign = doc->NewElement("Alignment");
			pAlign->SetAttribute("x", gui->alignMin.x);
			pAlign->SetAttribute("y", gui->alignMin.y);

			XMLElement* pAlignExt = doc->NewElement("AlignExt");
			pAlignExt->SetAttribute("x", gui->alignExt.x);
			pAlignExt->SetAttribute("y", gui->alignExt.y);

			pGUI->InsertFirstChild(pPos);
			pGUI->InsertAfterChild(pPos, pExt);
			pGUI->InsertAfterChild(pExt, pAlign);
			pGUI->InsertEndChild(pAlignExt);

			pNode->InsertEndChild(pGUI);
		}
		//Material Info
		if (parent->engineFlags & COMPONENT_MATERIAL) {
			MaterialComponent* m = (MaterialComponent*)parent->data->getComponent<MaterialComponent>();
			XMLElement* pMaterial = doc->NewElement("Material");
 			pMaterial->SetAttribute("ID", m->matUnqiueID);

			pNode->InsertEndChild(pMaterial);
		}

		//Light Info
		if (parent->engineFlags & COMPONENT_LIGHT) {
			LightComponent* l = (LightComponent*)parent->data->getComponent<LightComponent>();

			XMLElement* pColor = doc->NewElement("Color");
			pColor->SetAttribute("r", l->color.r);
			pColor->SetAttribute("g", l->color.g);
			pColor->SetAttribute("b", l->color.b);

			XMLElement* pIntensity = doc->NewElement("Intensity");
			pIntensity->SetAttribute("i", l->intensity);

			XMLElement* pID = doc->NewElement("ID");
			pID->SetAttribute("id", l->id);

			pNode->InsertEndChild(pColor);
			pNode->InsertEndChild(pIntensity);
			pNode->InsertEndChild(pID);
		}

		if (parent->engineFlags & COMPONENT_CAMERA)
		{
			CameraComponent* c = (CameraComponent*)parent->data->getComponent<CameraComponent>();

			XMLElement* pLookAt = doc->NewElement("AspectRatio");
			pLookAt->SetAttribute("ratio", c->aspectRatio);

			XMLElement* pFOV = doc->NewElement("FOV");
			pFOV->SetAttribute("fov", c->fov);

			pNode->InsertEndChild(pLookAt);
			pNode->InsertEndChild(pFOV);
		}

		if (parent->engineFlags & COMPONENT_PRIMITIVE) {
			PrimitiveComponent* obj = (PrimitiveComponent*)parent->data->getComponent<PrimitiveComponent>();
			XMLElement* pObj = doc->NewElement("Object");
			pObj->SetAttribute("ID", obj->id);
			pNode->InsertEndChild(pObj);
		}

		//Save rigidness
		if (parent->engineFlags & COMPONENT_RIGIDBODY) {
			XMLElement* pRigid = doc->NewElement("Rigid");
			pRigid->SetAttribute("Rigid", true);
			pNode->InsertEndChild(pRigid);
		}
		//if (parent->engineFlags & COMPONENT_CCONTROLLER) {
		//	//CharacterComponent* cont = (CharacterComponent*)parent->data->getComponent<CharacterComponent>();
		//	//ControllerComponent* roller = (ControllerComponent*)parent->data->getComponent<ControllerComponent>();
		//	//XMLElement* pController = doc->NewElement("Controller");
		//	//pController->SetAttribute("ControllerIndex", roller->index),
		//	//	pController->SetAttribute("d0", cont->speed);
		//	//pController->SetAttribute("d1", cont->maxSpeed);
		//	//pController->SetAttribute("d2", cont->jumpSpeed);
		//	//pController->SetAttribute("d3", cont->maxJumpHeight);
		//	//pNode->InsertEndChild(pController);
		//}
		if (parent->engineFlags & COMPONENT_COLIDER) {
			CollisionComponent* col = (CollisionComponent*)parent->data->getComponent<CollisionComponent>();

			XMLElement* pCollision = doc->NewElement("Collider");
			XMLElement* pLocal = doc->NewElement("Local");
			XMLElement* pExtents = doc->NewElement("Extents");

			pCollision->SetAttribute("Type", (int)col->type);
			pExtents->SetAttribute("x", col->extents.x);
			pExtents->SetAttribute("y", col->extents.y);
			pExtents->SetAttribute("z", col->extents.z);
			pLocal->SetAttribute("x", col->local.x);
			pLocal->SetAttribute("y", col->local.y);
			pLocal->SetAttribute("z", col->local.z);

			pCollision->InsertFirstChild(pLocal);
			pCollision->InsertEndChild(pExtents);
			pNode->InsertEndChild(pCollision);
		}
		if (parent->gameFlags > 0) {
			gameData->SaveGameData(parent, pNode, doc);
		}
		//if it has children, do that recursion stuff
		if (hasChildren) {
			for (NodeComponent* child : parent->children)
				pNode->InsertEndChild(saveNode(child, doc));
		}

		return pNode;
		
	}

	NodeComponent * SerializeNode::loadNode(tinyxml2::XMLElement * node)
	{
		artemis::Entity* e = &em->create();
		loadNode(node, e);
		return (NodeComponent*)e->getComponent<NodeComponent>();
	}

	void SerializeNode::loadNode(tinyxml2::XMLElement * node, artemis::Entity* e)
	{
		const char* name;
		bool hasChildren;
		int eFlags;
		int gFlags;
		bool dynamic;

		node->QueryStringAttribute("Name", &name);
		node->QueryBoolAttribute("hasChildren", &hasChildren);
		node->QueryIntAttribute("eFlags", &eFlags);
		node->QueryIntAttribute("gFlags", &gFlags);
		node->QueryBoolAttribute("Dynamic", &dynamic);

		dynamic ?  e->addComponent(new DynamicComponent) :  e->addComponent(new StaticComponent);

		NodeComponent* n = new NodeComponent(e, name, eFlags);
		n->isDynamic = dynamic;
		n->engineFlags = eFlags;
		n->gameFlags = gFlags;
		 e->addComponent(n); //TODO: aparently this might not be needed???? wat????

		 if (eFlags & COMPONENT_HEADNODE)
			 e->addComponent(new HeadNodeComponent());

		if (gFlags == 4) {
			// e->addComponent(new AudioComponent(dir + "Test/Audio/wallcol.wav"));
		}
		glm::vec3 transPos;
		if (eFlags & COMPONENT_TRANSFORM) {
			glm::vec3 pos;
			glm::vec3 rot;
			glm::vec3 sca;

			XMLElement* transform = node->FirstChildElement("Transform");

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

			transPos = pos;
			 e->addComponent(trans);
		}
		else {
			 e->addComponent(new TransformComponent());
			n->engineFlags |= COMPONENT_TRANSFORM;
		}
		if (eFlags & COMPONENT_PREFAB) {
			const char* name; const char* dir;
			bool save; bool load_needed; bool can_serialize;
			XMLElement* prefab_element  = node->FirstChildElement("Prefab");
			XMLElement* prefab_file		= prefab_element->FirstChildElement("FilePath");
			XMLElement* prefab_options	= prefab_element->FirstChildElement("PrefabOptions");
			
			prefab_file->QueryStringAttribute("name", &name);
			prefab_file->QueryStringAttribute("dir", &dir);
			prefab_options->QueryBoolAttribute("save", &save);
			prefab_options->QueryBoolAttribute("load", &load_needed);
			prefab_options->QueryBoolAttribute("serialize", &can_serialize);
			
			e->addComponent(new PrefabComponent(name, dir, save, load_needed, can_serialize));

		}
		if (eFlags & COMPONENT_GUI) {
			glm::vec2 pos;
			glm::vec2 ext;
			glm::vec2 align;
			glm::vec2 alignExt;

			XMLElement* gui = node->FirstChildElement("GUI");

			XMLElement* position = gui->FirstChildElement("Position");
			position->QueryFloatAttribute("x", &pos.x);
			position->QueryFloatAttribute("y", &pos.y);

			XMLElement* extents = gui->FirstChildElement("Extent");
			extents->QueryFloatAttribute("x", &ext.x);
			extents->QueryFloatAttribute("y", &ext.y);

			XMLElement* alignment = gui->FirstChildElement("Alignment");
			alignment->QueryFloatAttribute("x", &align.x);
			alignment->QueryFloatAttribute("y", &align.y);

			XMLElement* alignmentExtents = gui->FirstChildElement("AlignExt");
			alignmentExtents->QueryFloatAttribute("x", &alignExt.x);
			alignmentExtents->QueryFloatAttribute("y", &alignExt.y);
			if (gFlags == 8) {
				int goalNum = n->name.at(n->name.length() - 1) - '0';
				// e->addComponent(new BallScoreComponent(goalNum));
				GUINumberComponent* guinumber = new GUINumberComponent(pos, ext, 0);
				guinumber->alpha = 0;// false;
				 e->addComponent(guinumber);
				// e->addComponent(new AudioComponent(dir + "Test/Audio/goal.wav"));
			}
		}
		if (eFlags & COMPONENT_COLIDER) {
			glm::vec3 local;
			glm::vec3 extents;
			int type;

			XMLElement* c = node->FirstChildElement("Collider");
			XMLElement* l = c->FirstChildElement("Local");
			XMLElement* ex = c->FirstChildElement("Extents");

			c->QueryIntAttribute("Type", &type);

			l->QueryFloatAttribute("x", &local.x);
			l->QueryFloatAttribute("y", &local.y);
			l->QueryFloatAttribute("z", &local.z);

			ex->QueryFloatAttribute("x", &extents.x);
			ex->QueryFloatAttribute("y", &extents.y);
			ex->QueryFloatAttribute("z", &extents.z);

			eFlags & COMPONENT_TRANSFORM ?
				 e->addComponent(new CollisionComponent(transPos, local, extents, CollisionType(type)))
				:
				 e->addComponent(new CollisionComponent(local, extents, CollisionType(type)));
		}
		if (eFlags & COMPONENT_MATERIAL) {
			int matID;

			XMLElement* material = node->FirstChildElement("Material");
			material->QueryIntAttribute("ID", &matID);

			 e->addComponent(new MaterialComponent(RESOURCEMANAGER.getMaterialIndexU(matID), matID));
		}
		if (eFlags & COMPONENT_LIGHT) {

			XMLElement* color = node->FirstChildElement("Color");
			XMLElement* intensity = node->FirstChildElement("Intensity");
			XMLElement* id = node->FirstChildElement("ID");

			glm::vec3 c;
			float i;
			int idyo;

			color->QueryFloatAttribute("r", &c.r);
			color->QueryFloatAttribute("g", &c.g);
			color->QueryFloatAttribute("b", &c.b);

			intensity->QueryFloatAttribute("i", &i);
			id->QueryIntAttribute("id", &idyo);
			 e->addComponent(new LightComponent(c, i, idyo));
			 e->addComponent(new RenderComponent(RenderType::RENDER_LIGHT));
			NodeComponent* nc = (NodeComponent*) e->getComponent<NodeComponent>();
			nc->isParent = true;
		}
		if (eFlags & COMPONENT_CAMERA) {
			XMLElement* ratio = node->FirstChildElement("AspectRatio");
			XMLElement* fov = node->FirstChildElement("FOV");

			float r;
			float f;

			ratio->QueryFloatAttribute("ratio", &r);
			fov->QueryFloatAttribute("fov", &f);

			 e->addComponent(new CameraComponent(r, f));
			 e->addComponent(new RenderComponent(RenderType::RENDER_CAMERA));
			NodeComponent* nc = (NodeComponent*) e->getComponent<NodeComponent>();
			nc->isParent = true;
		}
		if (eFlags & COMPONENT_MODEL) {
			NodeComponent* nc = (NodeComponent*) e->getComponent<NodeComponent>();
			nc->isParent = true;
			int a = 4;
		}
		//if (eFlags & COMPONENT_MODEL) {
		//	XMLElement* Model = node->FirstChildElement("Model");
		//	int id;
		//	Model->QueryIntAttribute("ID", &id);
		//	 e->addComponent(new ModelComponent(id));
		//}
		//if (eFlags & COMPONENT_MESH) {
		//	XMLElement* Mesh = node->FirstChildElement("Mesh");
		//	int id, ri;
		//	Mesh->QueryIntAttribute("ID", &id);
		//	Mesh->QueryIntAttribute("ResourceIndex", &ri);
		//	 e->addComponent(new MeshComponent(id, ri));
		//}
		if (eFlags & COMPONENT_PRIMITIVE) {
			XMLElement* Object = node->FirstChildElement("Object");
			int id;
			Object->QueryIntAttribute("ID", &id);
			 e->addComponent(new PrimitiveComponent(id));
			 e->addComponent(new RenderComponent(RenderType::RENDER_PRIMITIVE));
		}
		if (eFlags & COMPONENT_AABB) {
			// e->addComponent(new AABBComponent());
		}
		if (eFlags & COMPONENT_RIGIDBODY) {
			//insertRigidBody(n);
		}
		if (eFlags & COMPONENT_PREFAB) {
			XMLElement* prefab = node->FirstChildElement("Prefab");
			std::string name;
			std::string dir;
			bool save = false, load = false,  can_serialize = false;
		}
		//if (eFlags & COMPONENT_CCONTROLLER) {
		//	//XMLElement* cont = node->FirstChildElement("Controller");
		//	//float data[4];
		//	//int index;
		//	//cont->QueryIntAttribute("ControllerIndex", &index),
		//	//	cont->QueryFloatAttribute("d0", &data[0]);
		//	//cont->QueryFloatAttribute("d1", &data[1]);
		//	//cont->QueryFloatAttribute("d2", &data[2]);
		//	//cont->QueryFloatAttribute("d3", &data[3]);

		//	// e->addComponent(new CharacterComponent(data));
		//	// e->addComponent(new ControllerComponent(index));

		//	////audio string stuff
		//	//std::string audioFile;
		//	//index == 0 ?
		//	//	audioFile = dir + "Test/Audio/player1.wav" :
		//	//	audioFile = dir + "Test/Audio/player2.wav";
		//	// e->addComponent(new AudioComponent(audioFile));

		//	//insertController(n);
		//}
		//if (eFlags & COMPONENT_BUTTON) {
		//	 e->addComponent(new ButtonComponent());
		//	 e->addComponent(new ButtonComponent());
		//	 e->addComponent(new CollisionComponent());
		//}
		////////////////// HAS CHILDREN ///////////////////
		if (gFlags > 0) {
			gameData->LoadGameData(n, node);
		}
		if (hasChildren) {
			n->children = loadNodes(node->FirstChildElement("Node"), n);
		}
	}

	std::vector<NodeComponent*> SerializeNode::loadNodes(tinyxml2::XMLElement * start, NodeComponent * p)
	{
		std::vector<NodeComponent*> nodes;
		while (start != nullptr) {
			NodeComponent* node = loadNode(start);
			node->parent = p;
			node->data->refresh();
			nodes.push_back(node);
			start = start->NextSiblingElement();
		}
		return nodes;
	}

}