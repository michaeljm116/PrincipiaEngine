#include "../pch.h"
#include "poseSystem.h"
#include "../tinyxml2/tinyxml2.h"
#include "../Utility/nodeComponent.hpp"
#include "../Utility/serialize-node.h"
#include "animationComponent.hpp"

using namespace tinyxml2;

Principia::PoseSystem::PoseSystem()
{
	addComponentType<PoseComponent>();
}

Principia::PoseSystem::~PoseSystem()
{
}

void Principia::PoseSystem::initialize()
{
	poseMapper.init(*world);
}

// Adding a pose component to an entity will create/modify an animation file
void Principia::PoseSystem::added(artemis::Entity & e)
{
	PoseComponent*	pose = poseMapper.get(e);

	//Initialize variables
	std::string	filename = pose->fileName + ".anim";
	XMLDocument	doc;
	XMLElement*	pNode;
	XMLNode*	pRoot;

	//Check if there's a file already
	//If there's no file make a new one, else start from end
	XMLError eResult = doc.LoadFile(filename.c_str());
	if (eResult == XML_ERROR_FILE_NOT_FOUND){
		pRoot = doc.NewElement("Root");
		pNode = doc.NewElement("Pose");
		doc.InsertFirstChild(pRoot);
	}
	else {
		pRoot = doc.FirstChild();
		pNode = doc.NewElement("Pose");
	}
	
	/* Format Should Be:
		<Pose Name="Name" NumParts="num">
			<Tran Id="">
				<Rot x="" y="" z="" w=""/>
				<Pos x="" y="" z=""/>
				<Sca x="" y="" z=""/>
			</Tran>
			<Tran Id=""> ... </Tran>
		</Pose>
	*/
	int numParts = static_cast<int>(pose->pose.size());
	pNode->SetAttribute("Name", pose->poseName.c_str());
	
	for (int i = 0; i < numParts; ++i) {
		int cn = pose->pose[i].first;
		sqt t = pose->pose[i].second;

		XMLElement* pTransform = doc.NewElement("Tran");
		pTransform->SetAttribute("CN", cn);

		XMLElement* pPosition = doc.NewElement("Pos");
		pPosition->SetAttribute("x", t.position.x);
		pPosition->SetAttribute("y", t.position.y);
		pPosition->SetAttribute("z", t.position.z);

		XMLElement* pRotation = doc.NewElement("Rot");
		pRotation->SetAttribute("x", t.rotation.x);
		pRotation->SetAttribute("y", t.rotation.y);
		pRotation->SetAttribute("z", t.rotation.z);
		pRotation->SetAttribute("w", t.rotation.w);

		XMLElement* pScale = doc.NewElement("Sca");
		pScale->SetAttribute("x", t.scale.x);
		pScale->SetAttribute("y", t.scale.y);
		pScale->SetAttribute("z", t.scale.z);

		pTransform->InsertFirstChild(pPosition);
		pTransform->InsertAfterChild(pPosition, pRotation);
		pTransform->InsertEndChild(pScale);

		pNode->InsertEndChild(pTransform);
	}

	//Save the Node
	pRoot->InsertEndChild(pNode);
	eResult = doc.SaveFile(filename.c_str());
	
	//remove component

	e.removeComponent<PoseComponent>();
}

void Principia::PoseSystem::processEntity(artemis::Entity & e)
{
}
