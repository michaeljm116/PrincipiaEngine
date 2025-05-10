#include "../pch.h"
#include "resourceManager.h"
#include "window.h"
#include "xxhash.hpp"
#include "bvhComponent.hpp"
#include "../Rendering/shaderStructures.hpp"

namespace Principia {
#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != tinyxml2::XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

	Resources::Resources() {
	}

	bool Resources::LoadConfig(std::string fileName)
	{

		//initialize and set defaults
		config.numControllersConfigs = 4;
		rController global{};
		rController keyboard_left{};
		rController keyboard_right{};
		rController  gamepad{};
		//rController joystick{};

		for (int i = 0; i < 16; ++i) {
			global.buttons[i] = 0;
			keyboard_left.buttons[i] = 0;
			keyboard_right.buttons[i] = 0;
		}


		global.axis[0] = GLFW_KEY_D;
		global.axis[1] = GLFW_KEY_SPACE;
		global.axis[2] = GLFW_KEY_W;
		global.axis[3] = GLFW_KEY_A;
		global.axis[4] = GLFW_KEY_LEFT_ALT;
		global.axis[5] = GLFW_KEY_S;
		global.buttons[0] = GLFW_KEY_LEFT_BRACKET;
		global.buttons[1] = GLFW_KEY_RIGHT_BRACKET; 
		global.buttons[2] = GLFW_KEY_F11;
		global.buttons[3] = GLFW_KEY_ESCAPE;
		global.buttons[4] = GLFW_KEY_BACKSLASH;

		keyboard_left.axis[0] = GLFW_KEY_D;
		keyboard_left.axis[1] = GLFW_KEY_W;
		keyboard_left.axis[2] = GLFW_KEY_A;
		keyboard_left.axis[3] = GLFW_KEY_S;
		keyboard_left.axis[4] = GLFW_KEY_SPACE;
		keyboard_left.axis[5] = GLFW_KEY_Q;
		keyboard_left.buttons[0] = GLFW_KEY_SPACE; 
		keyboard_left.buttons[1] = GLFW_KEY_E;
		keyboard_left.buttons[2] = GLFW_KEY_G;
		keyboard_left.buttons[3] = GLFW_KEY_F;
		keyboard_left.buttons[8] = GLFW_KEY_TAB;

		keyboard_right.axis[0] = GLFW_KEY_RIGHT;
		keyboard_right.axis[1] = GLFW_KEY_UP;
		keyboard_right.axis[2] = GLFW_KEY_LEFT;
		keyboard_right.axis[3] = GLFW_KEY_DOWN;
		keyboard_right.axis[4] = GLFW_KEY_SPACE;
		keyboard_right.axis[5] = GLFW_KEY_RIGHT_SHIFT;
		keyboard_right.buttons[0] = GLFW_KEY_RIGHT_CONTROL;
		keyboard_right.buttons[1] = GLFW_KEY_RIGHT_ALT;
		keyboard_right.buttons[2] = GLFW_KEY_SLASH;
		keyboard_right.buttons[3] = GLFW_KEY_HOME;

		/*
		#define GLFW_GAMEPAD_BUTTON_A               0
		#define GLFW_GAMEPAD_BUTTON_B               1
		#define GLFW_GAMEPAD_BUTTON_X               2
		#define GLFW_GAMEPAD_BUTTON_Y               3
		#define GLFW_GAMEPAD_BUTTON_LEFT_BUMPER     4
		#define GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER    5
		#define GLFW_GAMEPAD_BUTTON_BACK            6
		#define GLFW_GAMEPAD_BUTTON_START           7
		#define GLFW_GAMEPAD_BUTTON_GUIDE           8
		#define GLFW_GAMEPAD_BUTTON_LEFT_THUMB      9
		#define GLFW_GAMEPAD_BUTTON_RIGHT_THUMB     10
		#define GLFW_GAMEPAD_BUTTON_DPAD_UP         11
		#define GLFW_GAMEPAD_BUTTON_DPAD_RIGHT      12
		#define GLFW_GAMEPAD_BUTTON_DPAD_DOWN       13
		#define GLFW_GAMEPAD_BUTTON_DPAD_LEFT       14
		#define GLFW_GAMEPAD_BUTTON_LAST            GLFW_GAMEPAD_BUTTON_DPAD_LEFT

		#define GLFW_GAMEPAD_BUTTON_CROSS       GLFW_GAMEPAD_BUTTON_A
		#define GLFW_GAMEPAD_BUTTON_CIRCLE      GLFW_GAMEPAD_BUTTON_B
		#define GLFW_GAMEPAD_BUTTON_SQUARE      GLFW_GAMEPAD_BUTTON_X
		#define GLFW_GAMEPAD_BUTTON_TRIANGLE    GLFW_GAMEPAD_BUTTON_Y
		*/

		for (int i = 0; i < 16; ++i) {
			if (i < 6) {
				gamepad.axis[i] = static_cast<float>(i);
			}
			gamepad.buttons[i] = i;
		}

		config.controllerConfigs.push_back(global);
		config.controllerConfigs.push_back(keyboard_left);
		config.controllerConfigs.push_back(keyboard_right);
		config.controllerConfigs.push_back(gamepad);

		return true;
	}

	bool Resources::LoadDirectory(std::string directory)
	{
		for (const auto & p : std::filesystem::directory_iterator(directory)) {
			//std::cout << p << std::endl;
			if (!LoadPModel(p.path().string()))
				break;
		}
		return false;
	}
	bool Resources::LoadAnimations(std::string directory)
	{
		for (const auto & p : std::filesystem::directory_iterator(directory)) {
			if (!LoadPose(p.path().string(), p.path().stem().string()))
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

		int introLength = 0;
		int nameLength = 0;
		int numMesh = 0;
		int uniqueID = 0;
		int skeletonID = 0;
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

		//Read unique ID's
		binaryio.read(reinterpret_cast<char*>(&uniqueID), sizeof(int));

		//Get num meshes;
		binaryio.read(reinterpret_cast<char*>(&numMesh), sizeof(int));

		for (int i = 0; i < numMesh; ++i) {
			rMesh m;
			int meshNameLength = 0;
			int numVerts = 0;
			int numFaces = 0;
			int numNodes = 0;
			int meshID = 0;

			//name
			binaryio.read(reinterpret_cast<char*>(&meshNameLength), sizeof(int));
			for (int n = 0; n < meshNameLength; ++n) {
				binaryio.read(&c, sizeof(char));
				m.name.push_back(c);
			}
			//id
			binaryio.read(reinterpret_cast<char*>(&meshID), sizeof(int));

			//nums
			binaryio.read(reinterpret_cast<char*>(&numVerts), sizeof(int));
			binaryio.read(reinterpret_cast<char*>(&numFaces), sizeof(int));
			binaryio.read(reinterpret_cast<char*>(&numNodes), sizeof(int));

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
			//faces
			m.faces.reserve(numFaces);
			for (int t = 0; t < numFaces; ++t) {
				glm::ivec4 face;
				binaryio.read(reinterpret_cast<char*>(&face), sizeof(glm::ivec4));
				m.faces.emplace_back(face);
			}

			//bvh nodes
			m.bvh.reserve(numNodes);
			for (int b = 0; b < numNodes; ++b) {
				ssBVHNode node;
				binaryio.read(reinterpret_cast<char*>(&node), sizeof(ssBVHNode));
				m.bvh.emplace_back(node);
			}

			m.meshID = meshID;
			//add the model

			mod.meshes.push_back(m);
		}

		//Get number of shapes
		int numShapes = 0;
		binaryio.read(reinterpret_cast<char*>(&numShapes), sizeof(int));
		for (int i = 0; i < numShapes; ++i) {
			int shapeNameLength = 0;
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

		int numTransforms = 0;
		binaryio.read(reinterpret_cast<char*>(&numTransforms), sizeof(int));

		////Find out of skinned or not
		//binaryio.read(reinterpret_cast<char*>(&skinned), sizeof(bool));
		////if (skinned) binaryio.read(reinterpret_cast<char*>(&skeletonID), sizeof(int));

		////get skeleton info if skinned
		//if (skinned) {
		//	//binaryio.read(reinterpret_cast<char*>(&skeletonID), sizeof(int));
		//	LoadSkeleton(binaryio, mod.name );
		//	mod.skinned = true;
		//}

		binaryio.close();
		mod.uniqueID = uniqueID;
		//mod.skeletonID = skinned ? (skeletons.end() - 1)->id : 0;
		
		//check if its triangulated
		auto triCheck = fileName.substr(fileName.length() - 5, fileName.length());
		if (triCheck == "_t") mod.triangular = true;

		models.push_back(mod);
		return true;
	}

	bool Resources::LoadPose(std::string fileName, std::string prefabName)
	{
		// Initialize variables
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLElement* pRoot;
		tinyxml2::XMLNode* pNode;
		tinyxml2::XMLError eResult = doc.LoadFile(fileName.c_str());

		// Confirm if the thing exist
		if (eResult == tinyxml2::XML_ERROR_FILE_NOT_FOUND) return eResult;

		// Do the things
		pNode = doc.FirstChild();
		pRoot = doc.FirstChildElement("Root");

		//Iterate through the poses
		rPoseList pl; pl.name = prefabName; pl.hashVal = xxh::xxhash<32, char>(prefabName.c_str());
		tinyxml2::XMLElement* poseElement = pRoot->FirstChildElement("Pose");

		while (poseElement != nullptr) {
			//Get the name
			rPose pose;
			const char* name;
			poseElement->QueryStringAttribute("Name", &name);
			pose.name = name;
			pose.hashVal = xxh::xxhash<32, char>(name);

			//Iterate through the transforms
			tinyxml2::XMLElement* transElement = poseElement->FirstChildElement("Tran");

			while (transElement != nullptr) {
				int i;
				sqt t;
				transElement->QueryIntAttribute("CN", &i);

				tinyxml2::XMLElement* pos = transElement->FirstChildElement("Pos");
				tinyxml2::XMLElement* rot = transElement->FirstChildElement("Rot");
				tinyxml2::XMLElement* sca = transElement->FirstChildElement("Sca");

				pos->QueryFloatAttribute("x", &t.position.x);
				pos->QueryFloatAttribute("y", &t.position.y);
				pos->QueryFloatAttribute("z", &t.position.z);

				rot->QueryFloatAttribute("x", &t.rotation.x);
				rot->QueryFloatAttribute("y", &t.rotation.y);
				rot->QueryFloatAttribute("z", &t.rotation.z);
				rot->QueryFloatAttribute("w", &t.rotation.w);

				sca->QueryFloatAttribute("x", &t.scale.x);
				sca->QueryFloatAttribute("y", &t.scale.y);
				sca->QueryFloatAttribute("z", &t.scale.z);

				pose.pose.push_back(std::make_pair(i, t));
				transElement = transElement->NextSiblingElement("Tran");
			}	

			pl.poses.push_back(pose);
			poseElement = poseElement->NextSiblingElement("Pose");
		}
		
		poses.push_back(pl);
		return eResult == tinyxml2::XML_SUCCESS;
	}

	tinyxml2::XMLError Resources::SaveMaterials()
	{
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError eResult;
		tinyxml2::XMLNode * pRoot = doc.NewElement("Root");

		doc.InsertFirstChild(pRoot);
		for(const rMaterial& mat : materials)
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
			saveMat->SetAttribute("TextureID", mat.textureID);
			pRoot->InsertEndChild(saveMat);
		}

		eResult = doc.SaveFile(materialsFileName.c_str());
		XMLCheckResult(eResult);
		return eResult;
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
			int ti;

			first->QueryStringAttribute("Name", &name);
			first->QueryFloatAttribute("DiffuseR", &diff.r);
			first->QueryFloatAttribute("DiffuseG", &diff.g);
			first->QueryFloatAttribute("DiffuseB", &diff.b);

			first->QueryFloatAttribute("Reflective", &ref);
			first->QueryFloatAttribute("Roughness", &rough);
			first->QueryFloatAttribute("Transparency", &trans);
			first->QueryFloatAttribute("Refractive", &ri);
			first->QueryIntAttribute("TextureID", &ti);

			rMaterial mat = rMaterial(name, diff, ref, rough, trans, ri, ti);

			materials.push_back(mat);

			if (first != last)
				first = first->NextSiblingElement("Material");
			else
				lastOne = true;
		}
		//XMLCheckResult(eResult);
		return eResult;
	}

	tinyxml2::XMLError Resources::LoadMaterials(std::string file) {
		return LoadMaterials(file.c_str());
	}

	//const rPose & Resources::getPose(const std::string& prefabName, const std::string& poseName)
	//{      
	//	//Get da hashes
	//	int prefN = xxh::xxhash<32, char>(prefabName);
	//	int poseN = xxh::xxhash<32, char>(poseName);

	//	//compare and find;
	//	for (auto& pr : poses) {
	//		if (pr.hashVal == prefN) {
	//			for (auto& po : pr.poses) {
	//				if (po.hashVal == poseN)
	//					return po;
	//			}
	//		}
	//	}
	//	return rPose();
	//}
	const rPose & Resources::getPose(const int prefN, const int poseN)
	{
		//compare and find;
		for (auto& pr : poses) {
			if (pr.hashVal == prefN) {
				for (auto& po : pr.poses) {
					if (po.hashVal == poseN)
						return po;
				}
			}
		}
		return {};
	}

	const rPose& Resources::getPose(const std::string& prefabName, const std::string& poseName) {
		int prefN = xxh::xxhash<32, char>(prefabName);
		int poseN = xxh::xxhash<32, char>(poseName);
		return getPose(prefN, poseN);
	}

	const std::vector<rPose>& Resources::getPoses(const int prefN) {
		for (auto& pr : poses) {
			if (pr.hashVal == prefN)
				return pr.poses;
		}
		return {};// std::vector<rPose>();
	}

	const std::vector<rPose>& Resources::getPoses(const std::string& prefabName) {
		int prefN = xxh::xxhash<32, char>(prefabName);
		return getPoses(prefN);
	}

	bool Resources::has_pose(std::string&& prefab_name)
	{
		int prefab_hash = xxh::xxhash<32, char>(prefab_name);
		for (auto& pr : poses) {
			if (pr.hashVal == prefab_hash)
				return true;
		}
		return false;
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
		for (int i = static_cast<int>(fn.size()) - 1; i > 0; --i) {
			if (fn.at(i) == '/') {
				index = ++i;
				break;
			}
		}
		fn = fn.substr(index, fn.size());

		return fn;
	}
}