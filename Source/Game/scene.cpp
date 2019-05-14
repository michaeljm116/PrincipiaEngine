#pragma once
#include "scene.h"
#include "../tinyxml2/tinyxml2.h"

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != tinyxml2::XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

void Scene::init(artemis::World& w) {

	world = &w;
	em = world->getEntityManager();
	sm = world->getSystemManager();
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();
	ts = (TransformSystem*)sm->getSystem<TransformSystem>();
	ui = (EngineUISystem*)sm->getSystem<EngineUISystem>();
	ps = (PhysicsSystem*)sm->getSystem<PhysicsSystem>();
	as = (AnimationSystem*)sm->getSystem<AnimationSystem>();

	cc = (CharacterController*)sm->setSystem(new CharacterController());
	input = (ControllerSystem*)sm->setSystem(new ControllerSystem());

	LoadScene("Pong/Arena");
	//LoadScene("Level1/QuadsTest");
	//LoadScene("Level1/Scene1");

};

void Scene::doStuff() {
	rs->addNodes(parents);

	ui->setActiveNode(parents[0]);
	cc->camera.transform = (TransformComponent*)parents[0]->data->getComponent<TransformComponent>();
	cc->camera.component = (CameraComponent*)parents[0]->data->getComponent<CameraComponent>();

	for each (NodeComponent* node in parents) {
		//ts->recursiveTransform(node);
		ts->recursiveTransform(node);
		
		if (node->flags & COMPONENT_RIGIDBODY)
			insertRigidBody(node);
		if (node->flags & COMPONENT_CCONTROLLER)
			insertController(node);
		if (node->flags & COMPONENT_BUTTON) {
			node->data->refresh();
			//button->change(*node->data);
			input->change(*node->data);
			insertRigidBody(node->children[0]);
		}
		if (node->flags & COMPONENT_GUI && node->tags == 8) {
			GUINumberComponent* gnc = (GUINumberComponent*)node->data->getComponent<GUINumberComponent>();
			rs->addGuiNumber(gnc);
		}
	}

	//rs->buildBVH();
}

#pragma region Creation Functions
void Scene::createModel(rModel resource, std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca, bool dynamic) {

	//Add Mesh Component and make it a parent node
	artemis::Entity* entity = &em->create();
	TransformComponent* parentTransform = new TransformComponent(pos, rot, sca);
	NodeComponent* parent = new NodeComponent(entity, name, COMPONENT_MODEL | COMPONENT_TRANSFORM | COMPONENT_AABB | COMPONENT_PRIMITIVE);

	//rMesh* mesh = &RESOURCEMANAGER.getModelU(resource);
	entity->addComponent(new ModelComponent(RESOURCEMANAGER.getModelIndex(resource.uniqueID), resource.uniqueID));
	entity->addComponent(new PrimitiveComponent(resource.uniqueID));
	entity->addComponent(new AABBComponent(pos, sca));	//MeshAABB's point to the physics system
	entity->addComponent(parent);
	entity->addComponent(parentTransform);

	parent->isDynamic = dynamic;
	glm::mat4 parentPosition = glm::mat4(1.f);
	glm::mat4 parentScale = glm::mat4(1.f);
	glm::mat4 parentRotation = glm::mat4(1.f);

	entity->refresh();
	//ps->change(*entity);
	//ps->addNode(parent);
	
	//set up the subsets
	int i = 0;
	for (std::vector<rMesh>::const_iterator itr = resource.meshes.begin(); itr != resource.meshes.end(); itr++) {

		//Create Entity
		artemis::Entity* child = &em->create();

		//Set up subset data
		NodeComponent* childNode = new NodeComponent(child, parent);
		TransformComponent* childTransform = new TransformComponent();// resource.meshes[i].center, resource.meshes[i].extents);

		child->addComponent(childNode);
		child->addComponent(childTransform);

		child->addComponent(new MeshComponent(resource.uniqueID, i));
		child->addComponent(new PrimitiveComponent(resource.uniqueID + i));
		child->addComponent(new MaterialComponent(0));
		child->addComponent(new AABBComponent());	//SubsetAABB's point to the rendering system


		//childTransform->parentSM = &parentTransform->scaleM;
		//childTransform->parentRM = &parentTransform->rotationM;
		//childTransform->parentPM = &parentTransform->positionM;

		childNode->name = resource.meshes[i].name;// "Child " + std::to_string(i);
		childNode->flags |= COMPONENT_MESH | COMPONENT_MATERIAL | COMPONENT_AABB | COMPONENT_TRANSFORM | COMPONENT_PRIMITIVE;
		parent->children.push_back(childNode);
		rs->addNode(childNode);

		++i;
	}
	for (i = 0; i < resource.shapes.size(); ++i) {
		//Create Entity
		artemis::Entity* child = &em->create();

		//Set up subset data
		NodeComponent* childNode = new NodeComponent(child, parent);
		TransformComponent* childTransform = new TransformComponent(resource.shapes[i].center, resource.shapes[i].extents);

		child->addComponent(childNode);
		child->addComponent(childTransform);
		child->addComponent(new PrimitiveComponent(resource.shapes[i].type));
		child->addComponent(new MaterialComponent(0));
		child->addComponent(new AABBComponent()); //will this even be used???

		childNode->name = resource.shapes[i].name;
		childNode->flags |= COMPONENT_MATERIAL | COMPONENT_TRANSFORM | COMPONENT_PRIMITIVE;
		parent->children.push_back(childNode);
		rs->addNode(childNode);
	}

	//rs->addNode(parent);
	rs->updateObjectMemory();
	parents.push_back(parent);
	ts->recursiveTransform(parent);

	//if its animatable....
	if (resource.skeletonID > 0) {
		parent->flags |= COMPONENT_SKINNED;
		rSkeleton* skelly = nullptr;
		skelly = &RESOURCEMANAGER.getSkeletonID(resource.skeletonID);
		if (skelly != nullptr) {
			AnimationComponent* anim = new AnimationComponent(resource.skeletonID);
			anim->skeleton.index = RESOURCEMANAGER.getSkeletonIndex(skelly->id);
			for (int i = 0; i < skelly->joints.size(); ++i) {
				Joint j;
				j.parentIndex = skelly->joints[i].parentIndex;
				j.invBindPose = skelly->joints[i].invBindPose;
				j.transform = skelly->joints[i].transform;
				anim->skeleton.joints.push_back(j);
			}
			anim->channels.resize(anim->skeleton.joints.size());

			entity->addComponent(anim);
			entity->refresh();
			as->change(*entity);
		}
	}
}

//Types: SPHERE = -1, BOX = -2, CYLINDER = -3, PLANE = -4
artemis::Entity* Scene::createShape(std::string name, glm::vec3 pos, glm::vec3 scale, int matID, int type, bool dynamic)
{
	artemis::Entity* e = &em->create();
	NodeComponent*		parent = new NodeComponent(e, name, COMPONENT_MATERIAL | COMPONENT_TRANSFORM | COMPONENT_PRIMITIVE);
	TransformComponent* trans  = new TransformComponent(pos, glm::vec3(0.f), scale);
	
	e->addComponent(new PrimitiveComponent(type));
	e->addComponent(new MaterialComponent(matID));
	e->addComponent(trans);
	e->addComponent(parent);

	parent->isDynamic = dynamic;
	e->refresh();
	rs->addNode(parent);
	parents.push_back(parent);
	ts->recursiveTransform(parent);
	rs->updateObjectMemory();

	return e;
}
artemis::Entity* Scene::createGameShape(std::string name, glm::vec3 pos, glm::vec3 scale, int matID, int type, bool dynamic)
{
	artemis::Entity* e = &em->create();
	NodeComponent*		parent = new NodeComponent(e, name, COMPONENT_MATERIAL | COMPONENT_TRANSFORM | COMPONENT_PRIMITIVE);
	TransformComponent* trans = new TransformComponent(pos, glm::vec3(0.f), scale);

	e->addComponent(new PrimitiveComponent(type));
	e->addComponent(new MaterialComponent(matID));
	e->addComponent(trans);
	e->addComponent(parent);

	parent->isDynamic = dynamic;
	e->refresh();
	rs->addNode(parent);
	ts->recursiveTransform(parent);
	rs->updateObjectMemory();

	return e;
}

void Scene::insertController(NodeComponent * nc)
{

	if (!(nc->flags & COMPONENT_CCONTROLLER)) {
		nc->flags |= COMPONENT_CCONTROLLER;
		nc->data->addComponent(new CharacterComponent());
		nc->data->addComponent(new ControllerComponent(1));
	}

	ControllerComponent* controller = (ControllerComponent*)nc->data->getComponent<ControllerComponent>();
	for (int i = 0; i < NUM_BUTTONS; ++i) {
		controller->buttons[i].key = RESOURCEMANAGER.getConfig().controllerConfigs[controller->index][i];
	}

	nc->data->refresh();
	input->change(*nc->data);
	
	cc->change(*nc->data);
	cc->characterTransform = (TransformComponent*)nc->data->getComponent<TransformComponent>();
	cc->characterNode = nc;

	//sets up the singleton to also use this controller
	if (controller->index == 1) {
		ControllerComponent* scomp = new ControllerComponent(controller);
		world->getSingleton()->addComponent(scomp);
		world->getSingleton()->refresh();
		input->change(*world->getSingleton());
	}

}

void Scene::insertRigidBody(NodeComponent* nc) {
	if(!(nc->flags & COMPONENT_RIGIDBODY))
	nc->flags |= COMPONENT_RIGIDBODY;
	TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();

	nc->data->addComponent(new RigidBodyComponent(1.f, tc->world));
	nc->data->refresh();
	ps->change(*nc->data);
	ps->addNode(nc);

	if (nc->flags & COMPONENT_COLIDER) {
		//nc->data->addComponent(new CollisionComponent());
		//nc->data->refresh();
		cc->change(*nc->data);
	}
}

void Scene::insertGoal(artemis::Entity & e)
{

}

artemis::Entity* Scene::createLight() {//glm::vec3 pos, glm::vec3 color, float intensity) {
	artemis::Entity* e = &em->create();

	e->addComponent(new LightComponent());
	//LightComponent* l = new LightComponent();
	//l->color = color;
	//l->intensity = intensity;

	e->addComponent(new TransformComponent(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f)));
	e->addComponent(new NodeComponent(e));
	//ui->addParentEntity(e, "Light");
	NodeComponent* parent = (NodeComponent*)e->getComponent<NodeComponent>();
	parent->name = "Light";
	parent->flags |= COMPONENT_LIGHT | COMPONENT_TRANSFORM;
	parents.push_back(parent);
	rs->addLight(*e);

	return e;

}

artemis::Entity* Scene::createCamera() {// glm::vec3 pos) {
	artemis::Entity* e = &em->create();

	e->addComponent(new CameraComponent(glm::vec3(0.f), glm::vec3(0.f, 0.5f, 0.0f), 10.f));
	e->addComponent(new TransformComponent(glm::vec3(0), glm::vec3(0.f), glm::vec3(0.f)));
	e->addComponent(new NodeComponent(e));
	//ui->addParentEntity(e, "Camera");
	NodeComponent* parent = (NodeComponent*)e->getComponent<NodeComponent>();
	parent->name = "Camera";
	parent->flags |= COMPONENT_CAMERA | COMPONENT_TRANSFORM;
	parents.push_back(parent);
	rs->addCamera(*e);

	return e;
}


#pragma endregion functions for creating things and scenifying them

void Scene::deleteNode(std::vector<NodeComponent*>& nParents, int nIndex)
{
	NodeComponent* parent = nParents[nIndex];
	//First delete all children if haz childrenz
	if (parent->children.size() > 0)
		deleteAllChildren(parent);
	//delete stuff 
	rs->deleteNode(parent);
	ps->deleteNode(parent);
	em->remove(*parent->data);
	nParents.erase(nParents.begin() + nIndex);	
	rs->updateObjectMemory();
}
void Scene::deleteNode(NodeComponent* parent) {
	//delete stuff 
	rs->deleteNode(parent);
	ps->deleteNode(parent);
	em->remove(*parent->data);
	//nParents.erase(nParents.begin() + nIndex);
	rs->updateObjectMemory();
}

void Scene::copyNode(NodeComponent * node, NodeComponent* parent, std::vector<NodeComponent*>& list)
{
	//create entity
	artemis::Entity* e = &em->create();
	//create new node
	e->addComponent(new NodeComponent(e, parent, *node));
	NodeComponent* copy = (NodeComponent*)e->getComponent<NodeComponent>();

	//copy all the datas
	if (node->flags & COMPONENT_TRANSFORM) {
		e->addComponent(new TransformComponent(*(TransformComponent*)node->data->getComponent<TransformComponent>()));
	}
	else { //if this is gonna be a parent, it must have a transform componenet... right?
		if (parent == nullptr) {
			if (node->parent->data->getComponent<TransformComponent>() != nullptr)
				e->addComponent(new TransformComponent(*(TransformComponent*)node->parent->data->getComponent<TransformComponent>()));
			else
				e->addComponent(new TransformComponent(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f)));
			copy->flags |= COMPONENT_TRANSFORM;
		}
	}
	if (node->flags & COMPONENT_MATERIAL) {
		e->addComponent(new MaterialComponent(*(MaterialComponent*)node->data->getComponent<MaterialComponent>()));
	}
	if (node->flags & COMPONENT_LIGHT) {
		e->addComponent(new LightComponent(*(LightComponent*)node->data->getComponent<LightComponent>()));
	}
	if (node->flags & COMPONENT_PRIMITIVE) {
		e->addComponent(new PrimitiveComponent(*(PrimitiveComponent*)node->data->getComponent<PrimitiveComponent>()));
	}
	if (node->flags & COMPONENT_AABB) {
		e->addComponent(new AABBComponent(*(AABBComponent*)node->data->getComponent<AABBComponent>()));
	}
	if (node->flags & COMPONENT_RIGIDBODY) {
		insertRigidBody(copy);
	}
	//add the children
	if (node->children.size() > 0) {
		for each (NodeComponent* child in node->children)
		{
			copyNode(child, copy, copy->children);
		}
	}

	ts->recursiveTransform(copy);
	rs->addNode(copy);
	list.push_back(copy);
	rs->updateObjectMemory();
	//rs->updateMeshMemory();
}

void Scene::makeParent(NodeComponent * child)
{
	//Remove it from the parent
	for (auto c = child->parent->children.begin(); c != child->parent->children.end(); c++) {
		if (*c == child) {
			child->parent->children.erase(c);
			break;
		}
	}
	//
	if (!(child->flags & COMPONENT_TRANSFORM)) {
		child->data->addComponent(new TransformComponent(*(TransformComponent*)child->parent->data->getComponent<TransformComponent>()));
		child->flags |= COMPONENT_TRANSFORM;
	}
	//transform back to the thing
	//TransformComponent* c
	child->parent = nullptr;
	parents.push_back(child);
}

void Scene::makeChild(NodeComponent * node, NodeComponent * parent, std::vector<NodeComponent*>& list)
{
	//Remove it from the list you're taking it from
	for (auto c = list.begin(); c != list.end(); c++) {
		if (*c == node) {
			list.erase(c);
			break;
		}
	}
	// add it to the parent

	node->parent = parent;
	if (!(node->flags & COMPONENT_TRANSFORM)) {
		node->data->addComponent(new TransformComponent(*(TransformComponent*)node->parent->data->getComponent<TransformComponent>()));
		node->flags |= COMPONENT_TRANSFORM;
	}
	else {

		TransformComponent* pt = (TransformComponent*)parent->data->getComponent<TransformComponent>();
		TransformComponent* ct = (TransformComponent*)node->data->getComponent<TransformComponent>();

		//nt->parentPM = &pt->positionM;
		//nt->parentRM = &pt->rotationM;
		//nt->parentSM = &pt->scaleM;
		//nt->position = glm::vec3(glm::vec4(nt->position, 1.f) * glm::inverse(pt->positionM) * nt->positionM);
		//nt->rotation = glm::vec3(glm::vec4(nt->rotation, 1.f) * glm::inverse(pt->rotationM) * nt->rotationM);
		//nt->scale = glm::vec3(glm::vec4(nt->scale, 1.f) * glm::inverse(pt->scaleM) * nt->scaleM);
		ct->local.position -= pt->global.position;// -nt->position;
		ct->local.rotation *= glm::inverse(pt->global.rotation);// -nt->rotation;
		ct->local.scale /= pt->global.scale;// -nt->scale;
		//nt->world *= glm::inverse(pt->world);
		//nt->position *= glm::vec3(glm::vec4(nt->position, 1.f) * nt->world);
	}


	parent->children.push_back(node);
	//ts->recursiveTransform(node);
	ts->recursiveTransform(node);
}

void Scene::updateObject(NodeComponent * node)
{
	if (node->flags & COMPONENT_MESH)
		rs->setRenderUpdate(RenderSystem::UPDATE_MESH);
	if (node->flags & COMPONENT_SPHERE)
		rs->setRenderUpdate(RenderSystem::UPDATE_SPHERE);
	if (node->flags & COMPONENT_CYLINDER)
		rs->setRenderUpdate(RenderSystem::UPDATE_CYLINDER);
	if (node->flags & COMPONENT_BOX)
		rs->setRenderUpdate(RenderSystem::UPDATE_BOX);
	if (node->flags & COMPONENT_PLANE)
		rs->setRenderUpdate(RenderSystem::UPDATE_PLANE);
	if (node->flags & COMPONENT_LIGHT)
		rs->setRenderUpdate(RenderSystem::UPDATE_LIGHT);
}

#pragma region SAVE/LOADSCENE
using namespace tinyxml2;
XMLError Scene::SaveScene(std::string name)
{
	//First save the directory of the level u iz in
	//"../../Assets/Common/";

	XMLDocument doc;
	XMLError eResult;

	XMLNode * pRoot = doc.NewElement("Root");
	doc.InsertFirstChild(pRoot);
	XMLElement * sceneNumber = doc.NewElement("Scene");
	sceneNumber->SetAttribute("Num", 1);

	pRoot->InsertFirstChild(sceneNumber);

	for each (NodeComponent* node in parents)
	{
		XMLElement* element = saveNode(node, &doc);
		pRoot->InsertEndChild(element);
	}
	eResult = doc.SaveFile((dir+currentScene+".xml").c_str());
	XMLCheckResult(eResult);

}

XMLError Scene::LoadScene(std::string name)
{
	currentScene = name;
	XMLDocument doc;
	XMLError eResult = doc.LoadFile((dir+name+".xml").c_str());
	XMLNode * pNode = doc.FirstChild();

	XMLElement* pRoot = doc.FirstChildElement("Root");// ->FirstChildElement("Scene");
	XMLElement* sceneN = pRoot->FirstChildElement("Scene");
	sceneN->QueryIntAttribute("Num", &sceneNumber);

	XMLElement* first = pRoot->FirstChildElement("Node");
	XMLElement* last = pRoot->LastChildElement("Node");
	parents = loadNodes(first, last, nullptr);

	ui->setActiveAsCamera();
	return eResult;
}

XMLElement* Scene::saveNode(NodeComponent * parent, XMLDocument* doc)
{
	//Save name
	XMLElement * pNode = doc->NewElement("Node");
	pNode->SetAttribute("Name", parent->name.c_str());

	//find out if has children
	bool hasChildren = parent->children.size() > 0 ? true : false;
	pNode->SetAttribute("hasChildren", hasChildren);

	//Save node flags
	pNode->SetAttribute("Flags", (int64_t)parent->flags);

	//Save node tags
	pNode->SetAttribute("Tags", (int64_t)parent->tags);

	//Save dynamicness
	pNode->SetAttribute("Dynamic", parent->isDynamic);

	//save component infos
	//TransformInfo
	if (parent->flags & COMPONENT_TRANSFORM) {
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

		XMLElement* pScale	  = doc->NewElement("Scale");
		pScale->SetAttribute("x", t->local.scale.x);
		pScale->SetAttribute("y", t->local.scale.y);
		pScale->SetAttribute("z", t->local.scale.z);

		pTransform->InsertFirstChild(pPosition);
		pTransform->InsertAfterChild(pPosition, pRotation);
		pTransform->InsertEndChild(pScale);

		pNode->InsertEndChild(pTransform);
	}
	if (parent->flags & COMPONENT_GUI) {
		GUIComponent* gui = (GUIComponent*)parent->data->getComponent<GUIComponent>();
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
	if (parent->flags & COMPONENT_MATERIAL) {
		MaterialComponent* m = (MaterialComponent*)parent->data->getComponent<MaterialComponent>();
		XMLElement* pMaterial = doc->NewElement("Material");
		pMaterial->SetAttribute("ID", m->matUnqiueID);

		pNode->InsertEndChild(pMaterial);
	}

	//Light Info
	if (parent->flags & COMPONENT_LIGHT) {
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

	if (parent->flags & COMPONENT_CAMERA)
	{
		CameraComponent* c = (CameraComponent*)parent->data->getComponent<CameraComponent>();

		XMLElement* pLookAt = doc->NewElement("LookAt");
		pLookAt->SetAttribute("x", c->lookat.x);
		pLookAt->SetAttribute("y", c->lookat.y);
		pLookAt->SetAttribute("z", c->lookat.z);

		XMLElement* pFOV = doc->NewElement("FOV");
		pFOV->SetAttribute("fov", c->fov);

		pNode->InsertEndChild(pLookAt);
		pNode->InsertEndChild(pFOV);
	}

	if (parent->flags & COMPONENT_PRIMITIVE) {
		PrimitiveComponent* obj = (PrimitiveComponent*)parent->data->getComponent<PrimitiveComponent>();
		XMLElement* pObj = doc->NewElement("Object");
		pObj->SetAttribute("ID", obj->uniqueID);
		pNode->InsertEndChild(pObj);
	}

	//Save rigidness
	if (parent->flags & COMPONENT_RIGIDBODY) {
		XMLElement* pRigid = doc->NewElement("Rigid");
		pRigid->SetAttribute("Rigid", true);
		pNode->InsertEndChild(pRigid);
	}
	if (parent->flags & COMPONENT_CCONTROLLER) {
		CharacterComponent* cont = (CharacterComponent*)parent->data->getComponent<CharacterComponent>();
		ControllerComponent* roller = (ControllerComponent*)parent->data->getComponent<ControllerComponent>();
		XMLElement* pController = doc->NewElement("Controller");
		pController->SetAttribute("ControllerIndex", roller->index),
		pController->SetAttribute("d0", cont->speed);
		pController->SetAttribute("d1", cont->maxSpeed);
		pController->SetAttribute("d2", cont->jumpSpeed);
		pController->SetAttribute("d3", cont->maxJumpHeight);
		pNode->InsertEndChild(pController);
	}
	//if it has children, do that recursion stuff
	if (hasChildren) {
		for each (NodeComponent* child in parent->children)
			pNode->InsertEndChild(saveNode(child, doc));
	}

	return pNode;
}

std::vector<NodeComponent*> Scene::loadNodes(tinyxml2::XMLElement* start, tinyxml2::XMLElement* finish, NodeComponent* p) {
	std::vector<NodeComponent*> nodes;
	bool lastOne = false;
	// node
	while (!lastOne) {
		artemis::Entity* e = &em->create();

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

		NodeComponent* n = new NodeComponent(e, name, flags);
		n->isDynamic = dynamic;
		n->flags = flags;
		n->tags = tags;
		e->addComponent(n); //TODO: aparently this might not be needed???? wat????


		if (tags == 4) {
			e->addComponent(new AudioComponent(dir + "Pong/Audio/wallcol.wav"));
		}
	
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

			e->addComponent(trans);
		}
		else {
			e->addComponent(new TransformComponent());
			n->flags |= COMPONENT_TRANSFORM;
		}
		if (flags & COMPONENT_GUI) {
			glm::vec2 pos;
			glm::vec2 ext;
			glm::vec2 align;
			glm::vec2 alignExt;

			XMLElement* gui = start->FirstChildElement("GUI");

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
			if (tags == 8) {
				int goalNum = n->name.at(n->name.length() - 1) - '0';
				e->addComponent(new BallScoreComponent(goalNum));
				GUINumberComponent* guinumber = new GUINumberComponent(pos, ext, 0);
				guinumber->visible = false;
				e->addComponent(guinumber);
				e->addComponent(new AudioComponent(dir + "Pong/Audio/goal.wav"));
			}
		}
		if (flags & COMPONENT_MATERIAL) {
			int matID;

			XMLElement* material = start->FirstChildElement("Material");
			material->QueryIntAttribute("ID", &matID);
			
			e->addComponent(new MaterialComponent(RESOURCEMANAGER.getMaterialIndexU(matID),matID));
		}
		if (flags & COMPONENT_LIGHT) {

			XMLElement* color = start->FirstChildElement("Color");
			XMLElement* intensity = start->FirstChildElement("Intensity");
			XMLElement* id = start->FirstChildElement("ID");

			glm::vec3 c;
			float i;
			int idyo;

			color->QueryFloatAttribute("r", &c.r);
			color->QueryFloatAttribute("g", &c.g);
			color->QueryFloatAttribute("b", &c.b);

			intensity->QueryFloatAttribute("i", &i);
			id->QueryIntAttribute("id", &idyo);
			e->addComponent(new LightComponent(c, i, idyo));
		}
		if (flags & COMPONENT_CAMERA) {
			XMLElement* lookat = start->FirstChildElement("LookAt");
			XMLElement* fov = start->FirstChildElement("FOV");

			glm::vec3 l;
			float f;

			lookat->QueryFloatAttribute("x", &l.x);
			lookat->QueryFloatAttribute("y", &l.y);
			lookat->QueryFloatAttribute("z", &l.z);
			fov->QueryFloatAttribute("fov", &f);

			e->addComponent(new CameraComponent(l,f));
		}
		//if (flags & COMPONENT_MODEL) {
		//	XMLElement* Model = start->FirstChildElement("Model");
		//	int id;
		//	Model->QueryIntAttribute("ID", &id);
		//	e->addComponent(new ModelComponent(id));
		//}
		//if (flags & COMPONENT_MESH) {
		//	XMLElement* Mesh = start->FirstChildElement("Mesh");
		//	int id, ri;
		//	Mesh->QueryIntAttribute("ID", &id);
		//	Mesh->QueryIntAttribute("ResourceIndex", &ri);
		//	e->addComponent(new MeshComponent(id, ri));
		//}
		if (flags & COMPONENT_PRIMITIVE) {
			XMLElement* Object = start->FirstChildElement("Object");
			int id;
			Object->QueryIntAttribute("ID", &id);
			e->addComponent(new PrimitiveComponent(id));
		}
		if (flags & COMPONENT_AABB) {
			e->addComponent(new AABBComponent());
		}
		if (flags & COMPONENT_RIGIDBODY) {
			//insertRigidBody(n);
		}
		if(flags & COMPONENT_CCONTROLLER) {
			XMLElement* cont = start->FirstChildElement("Controller");
			float data[4];
			int index;
			cont->QueryIntAttribute("ControllerIndex", &index), 
			cont->QueryFloatAttribute("d0", &data[0]);
			cont->QueryFloatAttribute("d1", &data[1]);
			cont->QueryFloatAttribute("d2", &data[2]);
			cont->QueryFloatAttribute("d3", &data[3]);

			e->addComponent(new CharacterComponent(data));
			e->addComponent(new ControllerComponent(index + 1));

			//audio string stuff
			std::string audioFile;
			index == 0 ?
				audioFile = dir + "Pong/Audio/player1.wav" :
				audioFile = dir + "Pong/Audio/player2.wav";
			e->addComponent(new AudioComponent(audioFile));

			//insertController(n);
		}
		if (flags & COMPONENT_COLIDER) {
			e->addComponent(new CollisionComponent());
		}
		//if (flags & COMPONENT_BUTTON) {
		//	e->addComponent(new ButtonComponent());
		//	e->addComponent(new ButtonComponent());
		//	e->addComponent(new CollisionComponent());
		//}
		////////////////// HAS CHILDREN ///////////////////
		if (hasChildren) {
			n->children = loadNodes(start->FirstChildElement("Node"), start->LastChildElement("Node"), n);
		}
		///////////////////////////////////////////////////

		//loop it up
		n->parent = p;
		nodes.push_back(n);
		if (start != finish)
			start = start->NextSiblingElement();
		else
			lastOne = true;
	}
	return nodes;
}

#pragma endregion these are functions for saving and loading the scene

void Scene::deleteAllChildren(NodeComponent* parent)
{
	for each (NodeComponent* child in parent->children)
	{
		//first recursively delete all children before you delete all children
		if(child->children.size() > 0)
			deleteAllChildren(child);

		//remove from rendering
		rs->deleteNode(child);
		em->remove(*child->data);
	}
	parent->children.clear();
}
