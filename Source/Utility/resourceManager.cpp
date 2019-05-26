#include "resourceManager.h"
#include "../Rendering/model.h"
#include "window.h"
#include "../pch.h"

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != tinyxml2::XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

Resources::Resources() {
}

bool Resources::LoadConfig(std::string fileName)
{
	config.numControllersConfigs = 2;
	std::array<int, 24> global;
	std::array<int, 24> controller1;
	std::array<int, 24> controller2;
	for (int i = 0; i < 24; ++i) {
		global[i] = 0;
		controller1[i] = 0;
		controller2[i] = 0;
	}
	global[0] = GLFW_KEY_D;
	global[1] = GLFW_KEY_SPACE;
	global[2] = GLFW_KEY_W;
	global[3] = GLFW_KEY_A;
	global[4] = GLFW_KEY_LEFT_ALT;
	global[5] = GLFW_KEY_S;
	global[6] = GLFW_KEY_LEFT_BRACKET;
	global[7] = GLFW_KEY_F11;
	global[8] = GLFW_KEY_HOME;
	global[9] = GLFW_KEY_ESCAPE;

	controller1[0] = GLFW_KEY_D;
	controller1[1] = GLFW_KEY_W;
	controller1[2] = GLFW_KEY_A;
	controller1[3] = GLFW_KEY_S;
	controller1[4] = GLFW_KEY_SPACE;
	controller1[5] = GLFW_KEY_Q;
	controller1[6] = GLFW_KEY_F;
	controller1[7] = GLFW_KEY_G;
	controller1[8] = GLFW_KEY_E;
	controller1[9] = GLFW_KEY_HOME;

	//controller1[1] = GLFW_JOYSTICK_1;

	controller2[0] = GLFW_KEY_RIGHT;
	controller2[1] = GLFW_KEY_UP;
	controller2[2] = GLFW_KEY_LEFT;
	controller2[3] = GLFW_KEY_DOWN;
	controller2[4] = GLFW_KEY_SPACE;
	controller2[5] = GLFW_KEY_RIGHT_SHIFT;
	controller2[6] = GLFW_KEY_RIGHT_CONTROL;
	controller2[7] = GLFW_KEY_RIGHT_ALT;
	controller2[8] = GLFW_KEY_SLASH;
	controller2[9] = GLFW_KEY_HOME;

	config.controllerConfigs.push_back(global);
	config.controllerConfigs.push_back(controller1);
	config.controllerConfigs.push_back(controller2);

	return true;
}

bool Resources::LoadDirectory(std::string directory)
{
	for (const auto & p : fs::directory_iterator(directory)) {
		//std::cout << p << std::endl;
		if (!LoadPModel(p.path().string()))
			break;
	}
	return false;
}
bool Resources::LoadAnimations(std::string directory)
{
	for (const auto & p : fs::directory_iterator(directory)) {
		if (!LoadSkeleton(p.path().string()))
			break;
	}
	return false;
}

bool Resources::LoadResources(std::string fileName)
{
	XMLCheckResult(loadScript(fileName.c_str()));
	for (int i = 0; i < numMeshFiles; ++i) {
		if (!LoadPModel(fileNames[i]))
			break;
	}
	return true;
}

bool Resources::LoadPModel(std::string fileName)
{
	rModel mod;

	std::fstream binaryio;
	binaryio.open(fileName.c_str(), std::ios::in | std::ios::binary);

	int introLength;
	int nameLength;
	int numMesh;
	int uniqueID;
	int skeletonID;
	bool skinned = false;


	//dont rly need the intro but do it anyways
	binaryio.read(reinterpret_cast<char*>(&introLength), sizeof(int));
	char c;
	for (int i = 0; i < introLength; ++i)
		binaryio.read(&c, sizeof(c));

	//Read the name;
	binaryio.read(reinterpret_cast<char*>(&nameLength), sizeof(int));
	for (int i = 0; i < nameLength; ++i) {
		binaryio.read(&c, sizeof(c));
		mod.name.push_back(c);
	}

	//Find out of skinned or not
	binaryio.read(reinterpret_cast<char*>(&skinned), sizeof(bool));

	//Read unique ID's
	binaryio.read(reinterpret_cast<char*>(&uniqueID), sizeof(int));
	if (skinned) binaryio.read(reinterpret_cast<char*>(&skeletonID), sizeof(int));

	//Get num meshes;
	binaryio.read(reinterpret_cast<char*>(&numMesh), sizeof(int));

	for (int i = 0; i < numMesh; ++i) {
		rMesh m;
		int meshNameLength;
		int numVerts;
		int numFaces;

		//name
		binaryio.read(reinterpret_cast<char*>(&meshNameLength), sizeof(int));
		for (int n = 0; n < meshNameLength; ++n) {
			binaryio.read(&c, sizeof(char));
			m.name.push_back(c);
		}
		//nums
		binaryio.read(reinterpret_cast<char*>(&numVerts), sizeof(int));
		binaryio.read(reinterpret_cast<char*>(&numFaces), sizeof(int));

		//aabbs
		binaryio.read(reinterpret_cast<char*>(&m.center), sizeof(glm::vec3));
		binaryio.read(reinterpret_cast<char*>(&m.extents), sizeof(glm::vec3));

		//vertices
		m.verts.reserve(numVerts);
		for (int v = 0; v < numVerts; ++v) {
			rVertex vert;
			binaryio.read(reinterpret_cast<char*>(&vert), sizeof(rVertex));
			//m.verts.push_back(vert);
			m.verts.emplace_back(vert);
		}
		//tris
		m.faces.reserve(numFaces);
		for (int t = 0; t < numFaces; ++t) {
			glm::ivec4 face;
			binaryio.read(reinterpret_cast<char*>(&face), sizeof(glm::ivec4));
			m.faces.emplace_back(face);
		}
		
		//bones
		if (skinned) {
			int numBones;
			binaryio.read(reinterpret_cast<char*>(&numBones), sizeof(int));
			m.bones.reserve(numBones);
			for (int b = 0; b < numBones; ++b) {
				rJointData jd;
				binaryio.read(reinterpret_cast<char*>(&jd.id), sizeof(int) * 4);
				binaryio.read(reinterpret_cast<char*>(&jd.weights), sizeof(float) * 4);
				jd.average();
				m.bones.emplace_back(jd);
			}
		}
		//add the model
		mod.meshes.push_back(m);
	}

	//Get number of shapes
	int numShapes;
	binaryio.read(reinterpret_cast<char*>(&numShapes), sizeof(int));
	for (int i = 0; i < numShapes; ++i) {
		int shapeNameLength;
		rShape shape;
		binaryio.read(reinterpret_cast<char*>(&shapeNameLength), sizeof(int));
		for (int n = 0; n < shapeNameLength; ++n) {
			binaryio.read(&c, sizeof(char));
			shape.name.push_back(c);
		}
		binaryio.read(reinterpret_cast<char*>(&shape.type), sizeof(int));
		binaryio.read(reinterpret_cast<char*>(&shape.center), sizeof(glm::vec3));
		binaryio.read(reinterpret_cast<char*>(&shape.extents), sizeof(glm::vec3));

		mod.shapes.push_back(shape);
	}

	int numTransforms;
	binaryio.read(reinterpret_cast<char*>(&numTransforms), sizeof(int));




	binaryio.close();
	mod.uniqueID = uniqueID;
	mod.skeletonID = skinned ? skeletonID : 0;
	models.push_back(mod);
	return true;
}

bool Resources::LoadSkeleton(std::string fileName)
{
	rSkeleton skelly;

	std::fstream binaryio;
	binaryio.open(fileName.c_str(), std::ios::in | std::ios::binary);

	//dont rly need the intro but do it anyways
	int introLength;
	binaryio.read(reinterpret_cast<char*>(&introLength), sizeof(int));
	char c;
	for (int i = 0; i < introLength; ++i)
		binaryio.read(&c, sizeof(c));

	//Read the name;
	int nameLength;
	binaryio.read(reinterpret_cast<char*>(&nameLength), sizeof(int));
	for (int i = 0; i < nameLength; ++i) {
		binaryio.read(&c, sizeof(c));
		skelly.name.push_back(c);
	}

	//Read the UniqueID
	binaryio.read(reinterpret_cast<char*>(&skelly.id), sizeof(int));
	
	//Get numberofJoints
	int numJoints;
	binaryio.read(reinterpret_cast<char*>(&numJoints), sizeof(int));
	
	//Read the joints
	for (int j = 0; j < numJoints; ++j) {
		rJoint joint;
		//Get the name
		int jointNameLength;
		binaryio.read(reinterpret_cast<char*>(&jointNameLength), sizeof(int));
		for (int i = 0; i < jointNameLength; ++i) {
			binaryio.read(&c, sizeof(c));
			joint.name.push_back(c);
		}
		//get the parent index and bind pose
		binaryio.read(reinterpret_cast<char*>(&joint.parentIndex), sizeof(int));
		binaryio.read(reinterpret_cast<char*>(&joint.invBindPose), sizeof(glm::mat4));
		binaryio.read(reinterpret_cast<char*>(&joint.transform), sizeof(glm::mat4));
		skelly.joints.push_back(joint);
	}

	binaryio.read(reinterpret_cast<char*>(&skelly.globalInverseTransform), sizeof(glm::mat4));

	//Get number of animations
	int numAnims;
	binaryio.read(reinterpret_cast<char*>(&numAnims), sizeof(int));

	for (int a = 0; a < numAnims; ++a) {
		rAnimation anim;
		//idk why i saved the skeleton id for each anim but whatever
		binaryio.read(reinterpret_cast<char*>(&anim.skeletonID), sizeof(int));
		//get the name
		int animNameLength;
		binaryio.read(reinterpret_cast<char*>(&animNameLength), sizeof(int));
		for (int i = 0; i < animNameLength; ++i) {
			binaryio.read(&c, sizeof(c));
			anim.name.push_back(c);
		}

		//get the duration and the samples per second
		binaryio.read(reinterpret_cast<char*>(&anim.duration), sizeof(float));
		binaryio.read(reinterpret_cast<char*>(&anim.sps), sizeof(float));

		//NOW FOR THE CHANNELS DUN DUN DUN
		int numchannels;
		binaryio.read(reinterpret_cast<char*>(&numchannels), sizeof(int));
		for (int i = 0; i < numchannels; ++i) {
			rAnimChannel channel;
			int numKeys;
			binaryio.read(reinterpret_cast<char*>(&numKeys), sizeof(int));
			for (int k = 0; k < numKeys; k++) {
				rAnimKey key;
				binaryio.read(reinterpret_cast<char*>(&key.time), sizeof(float));
				binaryio.read(reinterpret_cast<char*>(&key.pos), sizeof(glm::vec3));
				binaryio.read(reinterpret_cast<char*>(&key.rot), sizeof(glm::quat));
				binaryio.read(reinterpret_cast<char*>(&key.sca), sizeof(glm::vec3));
				channel.keys.push_back(key);
			}
			anim.channels.push_back(channel);
			anim.numChannels = numchannels;
		}
		skelly.animations.push_back(anim);
	}

	binaryio.close();
	//skelly.buildGlobalBinds();
	skeletons.push_back(skelly);
	return true;
}

tinyxml2::XMLError Resources::SaveMaterials()
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError eResult;
	tinyxml2::XMLNode * pRoot = doc.NewElement("Root");

	doc.InsertFirstChild(pRoot);
	for each (rMaterial mat in materials)
	{
		tinyxml2::XMLElement * saveMat = doc.NewElement("Material");
		saveMat->SetAttribute("Name", mat.name.c_str());
		saveMat->SetAttribute("DiffuseR", mat.diffuse.r);
		saveMat->SetAttribute("DiffuseG", mat.diffuse.g);
		saveMat->SetAttribute("DiffuseB", mat.diffuse.b);
		saveMat->SetAttribute("Reflective", mat.reflective);
		saveMat->SetAttribute("Roughness", mat.roughness);
		saveMat->SetAttribute("Transparency", mat.transparency);
		saveMat->SetAttribute("Refractive", mat.refractiveIndex);
		pRoot->InsertEndChild(saveMat);
	}

	eResult = doc.SaveFile(materialsFileName.c_str());
	XMLCheckResult(eResult);
}
tinyxml2::XMLError Resources::LoadMaterials(const char* file) {
	materialsFileName = file;
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError eResult = doc.LoadFile(file);

	tinyxml2::XMLNode* pNode = doc.FirstChild();
	tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Root");
	tinyxml2::XMLElement* first = pRoot->FirstChildElement("Material");
	tinyxml2::XMLElement* last = pRoot->LastChildElement("Material");
	
	bool lastOne = false;
	while (!lastOne) {
		const char* name;
		glm::vec3 diff;
		float rough;
		float ref;
		float trans;
		float ri;

		first->QueryStringAttribute("Name", &name);
		first->QueryFloatAttribute("DiffuseR", &diff.r);
		first->QueryFloatAttribute("DiffuseG", &diff.g);
		first->QueryFloatAttribute("DiffuseB", &diff.b);

		first->QueryFloatAttribute("Reflective", &ref);
		first->QueryFloatAttribute("Roughness", &rough);
		first->QueryFloatAttribute("Transparency", &trans);
		first->QueryFloatAttribute("Refractive", &ri);

		rMaterial mat = rMaterial(name, diff, ref, rough, trans, ri);
		
		materials.push_back(mat);

		if (first != last)
			first = first->NextSiblingElement("Material");
		else
			lastOne = true;
	}
	XMLCheckResult(eResult);
}

tinyxml2::XMLError Resources::loadScript(const char* file) {
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError eResult = doc.LoadFile(file);
	XMLCheckResult(eResult);

	//tinyxml2::XMLNode * proot = doc.FirstChild();
	tinyxml2::XMLElement * pRoot = doc.FirstChildElement("Data");
	tinyxml2::XMLElement* initScript;

	initScript = pRoot->FirstChildElement("NumMeshFiles");
	initScript->QueryIntAttribute("n", &numMeshFiles);

	tinyxml2::XMLElement* meshElement;
	if (numMeshFiles) {
		initScript = pRoot->FirstChildElement("Meshes");
		meshElement = initScript->FirstChildElement("mesh");
		for (int i = 0; i < numMeshFiles; ++i) {
			if (i > 0)
				meshElement = meshElement->NextSiblingElement("mesh");
			const char* data;
			meshElement->QueryStringAttribute("m", &data);
			fileNames.push_back(data);
		}
	}
	
	return tinyxml2::XML_SUCCESS;

}

std::string Resources::nameMesh(std::string fn)
{
	fn = fn.substr(0, fn.size() - 4);
	int index;
	for (int i = fn.size() - 1; i > 0; --i) {
		if (fn.at(i) == '/') {
			index = ++i;
			break;
		}
	}
	fn = fn.substr(index, fn.size());

	return fn;
}
