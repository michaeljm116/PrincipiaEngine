#include "structs.h"
#include "helper.hpp"

namespace fs = std::filesystem;


// Skinned mesh class





bool DoTheImportThing(const std::string& pFile, PrincipiaModel& m, PrincipiaSkeleton& s, bool triangulate);

bool WritePEModel(PrincipiaModel& m, std::string fn);
bool WriteSkeleton(PrincipiaSkeleton& s, std::string fn);


PrincipiaModel ReadPEModel(const char* pFile);
bool SkeletonScaler(PrincipiaSkeleton& s);
bool LoadDirectory(std::string directory);


///////////////ANIMATION STUFF///////////////////////
struct tempDataStruct {
	std::map<std::string, bool> necessityMap;
	std::map<std::string, int> jointMap;
	std::vector<Joint> skeletonJoints;
	std::string skelename;
};

bool LoadBones(const aiScene* scene, tempDataStruct& tds);
aiNode* FindNodey(aiNode* node, const char* name);


void FindChildy(aiNode* node, int& i, tempDataStruct& tds);
void FindJointy(aiNode*, tempDataStruct& tds);
///////////////ANIMATION STUFF///////////////////////



enum MeshType {
	MESH_ONLY,
	SKINNED_MESH,
	SKIN_AND_ANIM
};

MeshType meshType = MESH_ONLY;

int main() {
	LoadDirectory("Input");
	system("Pause");
}

bool WritePEModel(PrincipiaModel& m, std::string fn) {
	std::fstream binaryio;
	//std::string fileName = fn;
	binaryio.open(fn, std::ios::out | std::ios::binary);

	//Intro
	std::string intro = "Principia Model File v1.0 created by Mike Murrell\n© 2018 Mike Murrell\nAll rights Reserved\n\n";
	int introLength = intro.length();
	binaryio.write(CCAST(&introLength), sizeof(int));
	binaryio.write(intro.c_str(), introLength);

	//Write Name of Model
	int modelNameLength = m.name.length();
	binaryio.write(CCAST(&modelNameLength), sizeof(int));
	binaryio.write(m.name.c_str(), modelNameLength);

	//Find out of skinned or not
	bool skinned = meshType != MESH_ONLY ? true : false;
	binaryio.write(CCAST(&skinned), sizeof(bool));

	//Insert UniqueID;
	binaryio.write(CCAST(&m.uniqueID), sizeof(int));

	//Insert SkeletonID;
	if (skinned)
		binaryio.write(CCAST(&m.skeletonID), sizeof(int));
	

	//Find numder of meshes
	int numMeshes = m.meshes.size();
	binaryio.write(CCAST(&numMeshes), sizeof(int));

	//For each mesh, do yo thang
	for (int i = 0; i < numMeshes; ++i) {
		int meshNameLength = m.meshes[i].name.length();
		int numVerts = m.meshes[i].vertices.size();
		int numTris = m.meshes[i].faces.size();

		//Name
		binaryio.write(CCAST(&meshNameLength), sizeof(int));
		binaryio.write(m.meshes[i].name.c_str(), meshNameLength);

		//Nums
		binaryio.write(CCAST(&numVerts), sizeof(int));
		binaryio.write(CCAST(&numTris), sizeof(int));

		//AABBs
		binaryio.write(CCAST(&m.meshes[i].center), sizeof(glm::vec3));
		binaryio.write(CCAST(&m.meshes[i].extent), sizeof(glm::vec3));

		for (int v = 0; v < numVerts; v++) {
			binaryio.write(CCAST(&m.meshes[i].vertices[v]), sizeof(Vertex));
		}
		for (int t = 0; t < numTris; t++) {
			binaryio.write(CCAST(&m.meshes[i].faces[t].v), sizeof(glm::ivec4));
		}

		/*if (skinned) {
			int numBones = m.meshes[i].bones.size();
			binaryio.write(CCAST(&numBones), sizeof(int));
			for (int b = 0; b < numBones; ++b) {
				binaryio.write(CCAST(&m.meshes[i].bones[b].IDs), sizeof(int) * 4);
				binaryio.write(CCAST(&m.meshes[i].bones[b].weights), sizeof(float) * 4);
			}
		}*/

	}

	//find number of shapes
	int numShapes = m.shapes.size();
	binaryio.write(CCAST(&numShapes), sizeof(int));
	for (int i = 0; i < numShapes; ++i) {
		int shapeNameLength = m.shapes[i].name.length();
		binaryio.write(CCAST(&shapeNameLength), sizeof(int));
		binaryio.write(m.shapes[i].name.c_str(), shapeNameLength);
		binaryio.write(CCAST(&m.shapes[i].type), sizeof(int));
		binaryio.write(CCAST(&m.shapes[i].center), sizeof(glm::vec3));
		binaryio.write(CCAST(&m.shapes[i].extents), sizeof(glm::vec3));

	}

	int numTransforms = 0;
	binaryio.write(CCAST(&numTransforms), sizeof(int));

	binaryio.close();

	return true;
}

bool WriteSkeleton(PrincipiaSkeleton& s, std::string fn) {
	std::fstream binaryio;
	//std::string fileName = fn;
	binaryio.open(fn, std::ios::out | std::ios::binary);

	//Intro
	std::string intro = "Principia Model File v1.0 created by Mike Murrell\n© 2018 Mike Murrell\nAll rights Reserved\n\n";
	int introLength = intro.length();
	binaryio.write(CCAST(&introLength), sizeof(int));
	binaryio.write(intro.c_str(), introLength);

	//Write Name of Skeleton
	int modelNameLength = s.name.length();
	binaryio.write(CCAST(&modelNameLength), sizeof(int));
	binaryio.write(s.name.c_str(), modelNameLength);

	//Insert UniqueID;
	binaryio.write(CCAST(&s.uniqueID), sizeof(int));

	//Insert Number of joints
	binaryio.write(CCAST(&s.numJoints), sizeof(int));

	for (int j = 0; j < s.numJoints; ++j) {
		int jointNameLength = s.joints[j].name.length();
		binaryio.write(CCAST(&jointNameLength), sizeof(int));
		binaryio.write(s.joints[j].name.c_str(), jointNameLength);
		binaryio.write(CCAST(&s.joints[j].parentIndex), sizeof(int));
		binaryio.write(CCAST(&s.joints[j].glInvBindPose), sizeof(glm::mat4)); //MEOW: THIS MAY NEED TO BE CONVERTED
		binaryio.write(CCAST(&s.joints[j].glTransform), sizeof(glm::mat4));
	}

	binaryio.write(CCAST(&s.globalInverseTransform), sizeof(glm::mat4));
	//Insert number of animations
	int numAnim = s.animations.size();
	binaryio.write(CCAST(&numAnim), sizeof(int));
	for (int a = 0; a < numAnim; ++a) {
		//Save skeleton id
		binaryio.write(CCAST(&s.uniqueID), sizeof(int));
		//Save name
		int animNameLength = s.animations[a].name.length();
		binaryio.write(CCAST(&animNameLength), sizeof(int));
		binaryio.write(s.animations[a].name.c_str(), animNameLength);
		
		//save duration and fps
		binaryio.write(CCAST(&s.animations[a].duration), sizeof(float));
		binaryio.write(CCAST(&s.animations[a].fps), sizeof(float));
	
		//save the channels =/
		//MEOW: THIS IS CURRENTLY ASSUMING THAT IT USES EVERY CHANNEL AND IS SORTED BY JOINT ALREADY
		binaryio.write(CCAST(&s.animations[a].numChannels), sizeof(int));
		for (int c = 0; c < s.animations[a].numChannels; c++) {
			binaryio.write(CCAST(&s.animations[a].channels[c].numKeys), sizeof(int));
			for (int k = 0; k < s.animations[a].channels[c].numKeys; ++k) {
				binaryio.write(CCAST(&s.animations[a].channels[c].keys[k].time), sizeof(float));
				binaryio.write(CCAST(&s.animations[a].channels[c].keys[k].pos.mValue), sizeof(aiVector3D));
				binaryio.write(CCAST(&aiQuatToGLM(s.animations[a].channels[c].keys[k].rot.mValue)), sizeof(glm::vec4));
				binaryio.write(CCAST(&s.animations[a].channels[c].keys[k].sca.mValue), sizeof(aiVector3D));
			}
		}
	}

	binaryio.close();
	return true;
}

PrincipiaModel ReadPEModel(const char* pFile){
	PrincipiaModel mod;
	std::fstream binaryio;
	binaryio.open(pFile, std::ios::in | std::ios::binary);

	int introLength;
	int nameLength;
	int numMesh;

	//dont rly need the intro but do it anyways
	binaryio.read(CCAST(&introLength), sizeof(int));
	char c;
	for (int i = 0; i < introLength; ++i)
		binaryio.read(&c, sizeof(c));

	//Read the name;
	binaryio.read(CCAST(&nameLength), sizeof(int));
	for (int i = 0; i < nameLength; ++i) {
		binaryio.read(&c, sizeof(c));
		mod.name.push_back(c);
	}

	
	//Get num meshes;
	binaryio.read(CCAST(&numMesh), sizeof(int));

	for (int i = 0; i < numMesh; ++i) {
		Mesh m;
		int meshNameLength;
		int numVerts;
		int numFaces;

		//name
		binaryio.read(CCAST(&meshNameLength), sizeof(int));
		for (int n = 0; n < meshNameLength; ++n) {
			binaryio.read(&c, sizeof(char));
			m.name.push_back(c);
		}
		//nums
		binaryio.read(CCAST(&numVerts), sizeof(int));
		binaryio.read(CCAST(&numFaces), sizeof(int));

		//aabbs
		binaryio.read(CCAST(&m.center), sizeof(glm::vec3));
		binaryio.read(CCAST(&m.extent), sizeof(glm::vec3));

		for (int v = 0; v < numVerts; ++v) {
			glm::vec3 vert;
			binaryio.read(CCAST(&vert), sizeof(glm::vec3));
			m.vertices.push_back(vert);
		}
		for (int t = 0; t < numFaces; ++t) {
			Face face;
			binaryio.read(CCAST(&face), sizeof(TriIndex));
			m.faces.push_back(face);
		}
		mod.meshes.push_back(m);
	}

	return mod;
}
bool DoTheImportThing(const std::string& pFile, PrincipiaModel& m, PrincipiaSkeleton& s, bool triangulate)
{
	tempDataStruct tds;
	// Create an instance of the Importer class
	Assimp::Importer importer;
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* pScene;//
	if (triangulate)
		pScene = importer.ReadFile(pFile,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);
	else
		pScene = importer.ReadFile(pFile,
			aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);// |
			//aiProcess_FindDegenerates | aiProcess_OptimizeMeshes );
	// If the import failed, report it


	//const aiNode* pRoot = pScene->mRootNode;
	//std::unordered_map<std::string, aiMatrix4x4> name_transformMap;
	//for (int i = 0; i < pRoot->mNumChildren; ++i)
	//	name_transformMap[pRoot->mChildren[i]->mName.C_Str()] = pRoot->mChildren[i]->mTransformation;

	if (!pScene)
	{
		std::string error = importer.GetErrorString();
		std::cout << "ERROR: " << error << std::endl;
		return false;
	}
	
	m.name = pScene->GetShortFilename(pFile.c_str());
	//shorten name
	int indexico;
	for (int i = 0; i < m.name.size(); ++i) {
		if (m.name.at(i) == '.') {
			indexico = i;
			break;
		}
	}
	m.name = m.name.substr(0, indexico);
	

	//LoadBones(pScene, tds);

	//get data mesh data
	int numtris = 0;
	int numquads = 0;
	for (int i = 0; i < pScene->mNumMeshes; ++i) {
		Mesh subset;
		glm::vec3 maxVert = glm::vec3(FLT_MIN);
		glm::vec3 minVert = glm::vec3(FLT_MAX);

		aiMesh* paiMesh = pScene->mMeshes[i];
		//aiMatrix4x4 trans = name_transformMap[paiMesh[i].mName.C_Str()];
		for (int v = 0; v < paiMesh->mNumVertices; ++v) {
			aiVector3D vert = paiMesh->mVertices[v];
			aiVector3D norm = paiMesh->mNormals[v];
			aiVector3D* txtr = nullptr;
			if(paiMesh->HasTextureCoords(v))
				txtr = paiMesh->mTextureCoords[v];

			//Transform the verts;
			

			//subset.vertices.push_back(glm::vec3(paiMesh->mVertices[v].x, paiMesh->mVertices[v].y, paiMesh->mVertices[v].z));
			paiMesh->HasTextureCoords(v) ? subset.vertices.push_back(Vertex(vert, norm, txtr->x, txtr->y)) : subset.vertices.push_back(Vertex(vert, norm));
			maxVert.x = maxVal(maxVert.x, vert.x);
			maxVert.y = maxVal(maxVert.y, vert.y);
			maxVert.z = maxVal(maxVert.z, vert.z);

			minVert.x = minVal(minVert.x, vert.x);
			minVert.y = minVal(minVert.y, vert.y);
			minVert.z = minVal(minVert.z, vert.z);		

		}
		for (int f = 0; f < paiMesh->mNumFaces; ++f) {
			//subset.faces.push_back(paiMesh->mFaces[f]);
			//TriIndex tri;
			Face face;
			for (int t = 0; t < paiMesh->mFaces[f].mNumIndices; t++) {
				if (t < 4)
					face.v[t] = paiMesh->mFaces[f].mIndices[t];
			}
			//this is to turn triangles into quads
			int numIndices = paiMesh->mFaces[f].mNumIndices;
			if (numIndices == 3) {
				numtris++;
				face.v[3] = face.v[2];
			}
			if (numIndices == 4)
				numquads++;

			subset.faces.push_back(face);


		}

		subset.center.x = (maxVert.x + minVert.x) * 0.5f;
		subset.center.y = (maxVert.y + minVert.y) * 0.5f;
		subset.center.z = (maxVert.z + minVert.z) * 0.5f;

		subset.extent.x = (maxVert.x - minVert.x) * 0.5f;
		subset.extent.y = (maxVert.y - minVert.y) * 0.5f;
		subset.extent.z = (maxVert.z - minVert.z) * 0.5f;

		subset.name = paiMesh->mName.C_Str();

		ShapeType type = ShapeCheck(subset.name);
		type == ShapeType::MESH ? m.meshes.push_back(subset) 
							    : m.shapes.push_back(ShapeCreate(subset, type));

	///////////////////////////////////////////////////TEST SKELETON TEST SKELETON//////////////////////////////////////////////
	///////////////////////////////////////////////////TEST SKELETON TEST SKELETON//////////////////////////////////////////////
	///////////////////////////////////////////////////TEST SKELETON TEST SKELETON//////////////////////////////////////////////
	//	std::vector<tempRJoint> rayJoints;
	//	if (paiMesh->mNumBones > 0) {
	//		std::vector<VertexBoneData> bonesy;
	//		m.meshes[i].bones.resize(paiMesh->mNumVertices);

	//		for (int b = 0; b < paiMesh->mNumBones; ++b) {
	//			aiBone* boney = paiMesh->mBones[b];
	//			//std::vector<aiVertexWeight> weights;
	//			tempRJoint rj;
	//			for (int w = 0; w < boney->mNumWeights; w++) {
	//				aiVertexWeight weighty = boney->mWeights[w];
	//				//m.meshes[i].bones[weighty.mVertexId].add(tds.jointMap.find(boney->mName.data)->second, weighty.mWeight);
	//				//weights.push_back(weighty);
	//					if (weighty.mWeight > 0.5f){
	//						rj.verts.push_back(weighty.mVertexId);
	//				}
	//			}
	//			rj.name = boney->mName.C_Str();
	//			rayJoints.push_back(rj);				
	//			//skelly.joints.push_back(jointy);
	//		}
	//		int total = 0;
	//		for (int i = 0; i < rayJoints.size(); ++i)
	//			total += rayJoints[i].verts.size();
	//		int a = 0;
	//		//m.meshes[i].v

	//		//skellys.push_back(skelly);
	//		/*for (int f = 0; f < paiMesh->mNumVertices; ++f) {
	//			m.meshes[i].vertices[f].boneData = bonesy[f];
	//		}*/
	//	}
	//}

	//if (pScene->HasAnimations() && meshType == SKINNED_MESH) {
	//	meshType = SKIN_AND_ANIM;
	//	aiMatrix4x4 glit = pScene->mRootNode->mTransformation;
	//	glit.Inverse();
	//	s.globalInverseTransform = aiMatrix4x4ToGlm(glit);
	//	s.joints = tds.skeletonJoints;
	//	s.numJoints = tds.skeletonJoints.size();
	//	int a = 0;
	//	for (; a < pScene->mNumAnimations; ++a) {
	//		aiAnimation* anim = pScene->mAnimations[a];
	//		PrincipiaAnimation animation;
	//		animation.name = anim->mName.data;
	//		animation.duration = anim->mDuration;
	//		animation.fps = anim->mTicksPerSecond;
	//		animation.skeletonID = s.uniqueID;

	//		for (int j = 0; j < s.joints.size(); ++j) {
	//			for (int ch = 0; ch < anim->mNumChannels; ch++) {
	//				aiNodeAnim* ana = anim->mChannels[ch];
	//				if (!::strcmp(ana->mNodeName.data, s.joints[j].name.c_str())) {
	//					AnimChannel channel;
	//					channel.name = ana->mNodeName.data;// anim->mName.data;
	//					if ((ana->mNumPositionKeys != ana->mNumRotationKeys) || (ana->mNumPositionKeys != ana->mNumScalingKeys) || (ana->mNumRotationKeys != ana->mNumScalingKeys))
	//						std::cout << "ERROR, UNEVEN KEYS\n";

	//					for (int k = 0; k < ana->mNumRotationKeys; k++) {
	//						KeySQT key;
	//						key.pos = ana->mPositionKeys[k];
	//						key.rot = ana->mRotationKeys[k];
	//						key.sca = ana->mScalingKeys[k];
	//						key.time = ana->mRotationKeys[k].mTime;
	//						channel.keys.push_back(key);
	//					}
	//					channel.numKeys = channel.keys.size();
	//					animation.channels.push_back(channel);
	//				}
	//				animation.numChannels = animation.channels.size();
	//			}
	//		}
	//		s.animations.push_back(animation);
	//	}
	}
	//s.name = tds.skelename;
	
	// We're done. Everything will be cleaned up by the importer destructor
	auto root = pScene->mRootNode;
	std::vector<std::string> names;
	std::vector<aiMatrix4x4> transforms;
	for (int i = 0; i < root->mNumChildren; ++i) {
		transforms.push_back(root->mChildren[i]->mTransformation);
		names.push_back(root->mChildren[i]->mName.C_Str());
	}
	return true;
}


bool SkeletonScaler(PrincipiaSkeleton& s) {
	//So first you have to find the global transform....
	//then you get hte greatest extents for the globe, then for every local you have to divide by that ratio?
	float maxE = FLT_MIN;
	for (int j = 0; j < s.joints.size(); ++j) {
		for (int i = 0; i < 3; ++i) {
			maxE = maxVal(maxE, abs(s.joints[j].glTransform[i][3]));
		}
	}

	float ratio = 1 / maxE;
	for (int j = 0; j < s.joints.size(); ++j) {
		for (int i = 0; i < 3; ++i) {
			s.joints[j].glTransform[i][3] *= maxE;
		}
	}

	return true;
}

bool LoadDirectory(std::string directory)
{
	for (const auto & p : fs::directory_iterator(directory)) {
		UID++;
		PrincipiaModel mod;
		PrincipiaSkeleton skeleton;
		mod.uniqueID = newUniqueID();
		skeleton.uniqueID = newUniqueID();
		mod.skeletonID = skeleton.uniqueID;
		bool triangulate = false;
		if (DoTheImportThing(p.path().string(), mod, skeleton, triangulate)) {
			//break;
			ModelScaler(mod);
			////////////////////////////////////////////////////////////////////////////////////////////////////UNMESHONLYIFYTHIS////////////////////////////////////////////////
			meshType = MESH_ONLY;
			if (triangulate)
				mod.name += "_t";
			WritePEModel(mod, "Output/" + mod.name + ".pm");

			if (meshType == SKIN_AND_ANIM) {
				skeleton.name = mod.name + "_skel";
				//SkeletonScaler(skeleton);
				WriteSkeleton(skeleton, "Output/" + skeleton.name + ".pm");
			}
		}
		meshType = MESH_ONLY;
	}
	return false;
}


#pragma region skeletonbiulding
bool LoadBones(const aiScene* scene, tempDataStruct& tds) {
	std::map<std::string, uint32_t> boneMapping;
	std::vector<aiNode*> boneNodes;
	std::vector<aiNode*> parentNodes;

	
	//build skeleton map here
	//preinitialize a necessity map with a "No"
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		uint32_t bmi = 0;
		aiMesh* meshy = scene->mMeshes[i];
		aiNode* nodey;
		for (int b = 0; b < meshy->mNumBones; ++b) {

			meshType = SKINNED_MESH;
			aiBone* boney = meshy->mBones[b];
			std::string name = boney->mName.data;
			tds.necessityMap[name] = false;


			//Find the "yeses by comparing map naems with scene names
			aiNode* nodey = FindNodey(scene->mRootNode, boney->mName.data);
			
			if (nodey != NULL) {
				tds.necessityMap[boney->mName.data] = true;
				boneNodes.push_back(nodey);
			}
		}
		if (meshType != SKINNED_MESH)
			return false;
	}
	//get the parent node, yes this is vry slow ONSquared
	for (int i = 0; i < boneNodes.size(); ++i) {
		bool copy = false;
		for (int j = 0; j < boneNodes.size(); ++j) {
			if (!::strcmp(boneNodes[i]->mParent->mName.data,boneNodes[j]->mName.data)) {
				copy = true;
			}
		}
		if (!copy)
			parentNodes.push_back(boneNodes[i]);
	}
	tds.skelename = parentNodes[0]->mParent->mName.data;

	//So basically make it so that for each parent it goes like parent indexs are 123456 etc... 
	//then list all the children from the start of the parent so you're flattening the list and remember the parent index
	for (int i = 0; i < parentNodes.size(); ++i) {
		tds.jointMap[parentNodes[i]->mName.data] = i;
		Joint j;
		j.name = parentNodes[i]->mName.data;
		j.parentIndex = -1;
		j.transform = parentNodes[i]->mTransformation;
		tds.skeletonJoints.push_back(j);
	}
	int index = tds.jointMap.size() - 1;
	//Find the children indexes
	for (int i = 0; i < parentNodes.size(); ++i) {
		FindChildy(parentNodes[i], index, tds);
	}
	//Sort them
	for (int i = parentNodes.size(); i < tds.jointMap.size(); ++i) {
		for (auto m = tds.jointMap.begin(); m != tds.jointMap.end(); m++) {
			if (i == m->second) {
				Joint j;
				j.name = m->first.c_str();
				for (int b = 0; b < boneNodes.size(); b++) {
					if (!::strcmp(boneNodes[b]->mName.C_Str(), j.name.c_str()))
						j.transform = boneNodes[b]->mTransformation;
				}
				tds.skeletonJoints.push_back(j);
			}
		}
	}
	//find the parent indexes
	for (int i = 0; i < parentNodes.size(); ++i)
		FindJointy(parentNodes[i], tds);

	//So now you have an organized list started with teh parent and havig all the parent indexes,
	//as well as a map of all the joints thats unorganized but who currs bout unorgmap
	//NOW for each bone you want to compare name with skeleton joint name and if == then put in the matrix
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh* meshy = scene->mMeshes[i];
		for (int b = 0; b < meshy->mNumBones; ++b) {
			aiBone* boney = meshy->mBones[b];
			for (int j = 0; j < tds.skeletonJoints.size(); ++j) {
				if (!::strcmp(boney->mName.data, tds.skeletonJoints[j].name.c_str())){
					tds.skeletonJoints[j].invBindPose = boney->mOffsetMatrix;
				}
			}
		}
	}

	for (int i = 0; i < tds.skeletonJoints.size(); ++i) {
		tds.skeletonJoints[i].invBindPose.Transpose();// = aiMatrix4x4::Transpose(skeletonJoints[i].invBindPose);
		tds.skeletonJoints[i].transform.Transpose();
		tds.skeletonJoints[i].glInvBindPose = aiMatrix4x4ToGlm(tds.skeletonJoints[i].invBindPose);
		tds.skeletonJoints[i].glTransform = aiMatrix4x4ToGlm(tds.skeletonJoints[i].transform);
	}

	//system("Pause");
	return true;

}
aiNode* FindNodey(aiNode* node, const char* name) {
	if (!::strcmp(node->mName.data, name))return node;
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		aiNode* const p = FindNodey(node->mChildren[i], name);// node->mChildren[i]->FindNodey(name);
		if (p) {
			return p;
		}
	}
	// there is definitely no sub-node with this name
	return nullptr;
}

//SO you have the parents...
/*
but what you wnat to do is... for ever child... tell it the index of the parent
and then give itself a new index
then call its child
tell its child its' index
give its child a new index
*/
void FindChildy(aiNode* node, int& i, tempDataStruct& tds) {
	if (tds.necessityMap.find(node->mName.data) != tds.necessityMap.end()) {
		std::pair<std::string, int> j = { node->mName.data, i };
		tds.jointMap.insert(j);
		i++;
	}
	for(int c = 0; c < node->mNumChildren; c++)
	FindChildy(node->mChildren[c], i, tds);
}

void FindJointy(aiNode* node, tempDataStruct& tds) {

	if (tds.necessityMap.find(node->mName.data) != tds.necessityMap.end()) {
		if (tds.jointMap.find(node->mParent->mName.data) != tds.jointMap.end()) {
			auto j = tds.jointMap.find(node->mParent->mName.data);
			Joint jointy;
			jointy.parentIndex = j->second;
			tds.skeletonJoints[tds.jointMap.find(node->mName.data)->second].parentIndex = j->second;
		}
	}

	for (int c = 0; c < node->mNumChildren; c++)
		FindJointy(node->mChildren[c], tds);
}
#pragma endregion
