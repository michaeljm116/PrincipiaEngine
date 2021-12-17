#include "structs.h"
#include "bvh.hpp"
#include "helper.hpp"
#include <queue>
namespace fs = std::filesystem;


// Skinned mesh class





bool DoTheImportThing(const std::string& pFile, PrincipiaModel& m, PrincipiaSkeleton& s, bool triangulate);

bool WritePEModel(PrincipiaModel& m, std::string fn);
bool WriteSkeleton(PrincipiaSkeleton& s, std::string fn);


PrincipiaModel ReadPEModel(const char* pFile);
bool LoadDirectory(std::string directory, bool triangulate);


///////////////ANIMATION STUFF///////////////////////
struct tempDataStruct {
	std::vector<Joint> skeletonJoints;

	std::vector<int> parentIndexes;
	std::vector<aiNode*> confirmedNodes;
	std::unordered_map<std::string, int> nodeIndexes;
	std::string skelename;
	std::vector<aiBone*> bones;
};

bool LoadBones(const aiScene* scene, tempDataStruct& tds);
///////////////ANIMATION STUFF///////////////////////

bool hasAnim = false;

int main() {
	std::cout << "Triangulate: y/n? " << std::endl;
	char triangulate;
	std::cin >> triangulate;
	bool t = false;
	if (triangulate == 'y') t = true;
	LoadDirectory("Input", t);
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
	
	//Insert UniqueID;
	binaryio.write(CCAST(&m.uniqueID), sizeof(uint32_t));
		

	//Find numder of meshes
	int numMeshes = m.meshes.size();
	binaryio.write(CCAST(&numMeshes), sizeof(int));

	//For each mesh, do yo thang
	for (int i = 0; i < numMeshes; ++i) {
		int meshNameLength = m.meshes[i].name.length();
		int numVerts = m.meshes[i].vertices.size();
		int numTris = m.meshes[i].faces.size();
		int numNodes = m.meshes[i].bvh.size();
		//int meshIndex = m.meshes
		//asdfadsf
		//Name
		binaryio.write(CCAST(&meshNameLength), sizeof(int));
		binaryio.write(m.meshes[i].name.c_str(), meshNameLength);

		//ID
		binaryio.write(CCAST(&m.meshes[i].id), sizeof(int));

		//Nums
		binaryio.write(CCAST(&numVerts), sizeof(int));
		binaryio.write(CCAST(&numTris), sizeof(int));
		binaryio.write(CCAST(&numNodes), sizeof(int));

		//AABBs
		binaryio.write(CCAST(&m.meshes[i].center), sizeof(glm::vec3));
		binaryio.write(CCAST(&m.meshes[i].extent), sizeof(glm::vec3));

		for (int v = 0; v < numVerts; v++) {
			binaryio.write(CCAST(&m.meshes[i].vertices[v]), sizeof(Vertex));
		}
		for (int t = 0; t < numTris; t++) {
			binaryio.write(CCAST(&m.meshes[i].faces[t].v), sizeof(glm::ivec4));
		}

		//BVHs
		for (auto n : m.meshes[i].bvh) {
			binaryio.write(CCAST(&n), sizeof(flatBVHNode));
		}
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

	//Write Name of Skeleton
	int modelNameLength = s.name.length();
	binaryio.write(CCAST(&modelNameLength), sizeof(int));
	binaryio.write(s.name.c_str(), modelNameLength);

	//Insert UniqueID;
	binaryio.write(CCAST(&s.uniqueID), sizeof(uint32_t));

	//Insert Number of joints
	binaryio.write(CCAST(&s.numJoints), sizeof(int));

	for (int j = 0; j < s.numJoints; ++j) {
		Joint& sj = s.joints[j];
		int jointNameLength = sj.name.length();
		binaryio.write(CCAST(&jointNameLength), sizeof(int));
		binaryio.write(sj.name.c_str(), jointNameLength);
		binaryio.write(CCAST(&sj.parentIndex), sizeof(int));
		binaryio.write(CCAST(&sj.glInvBindPose), sizeof(glm::mat4)); //MEOW: THIS MAY NEED TO BE CONVERTED
		
		binaryio.write(CCAST(&sj.glGlobalTransform), sizeof(glm::mat4));
		//binaryio.write(CCAST(&sj.glTransform), sizeof(glm::mat4));

		binaryio.write(CCAST(&sj.center), sizeof(glm::vec3));
		binaryio.write(CCAST(&sj.extents), sizeof(glm::vec3));
		
		int numVerts, numFaces, numShapes, numNodes;
		numVerts = sj.verts.size();
		numFaces = sj.faces.size();
		numShapes = sj.shapes.size();
		
		binaryio.write(CCAST(&numVerts), sizeof(int));
		binaryio.write(CCAST(&numFaces), sizeof(int));
		binaryio.write(CCAST(&numShapes), sizeof(int));

		for (auto v : sj.verts)
			binaryio.write(CCAST(&v), sizeof(Vertex));
		for (auto f : sj.faces)
			binaryio.write(CCAST(&f), sizeof(Face));
		for (auto m : sj.meshIds)
			binaryio.write(CCAST(&m), sizeof(uint32_t));
		for (auto s : sj.shapes) {
			int shapeNameLength = s.name.length();
			binaryio.write(CCAST(&shapeNameLength), sizeof(int));
			binaryio.write(s.name.c_str(), shapeNameLength);
			binaryio.write(CCAST(&s.type), sizeof(int));
			binaryio.write(CCAST(&s.center), sizeof(glm::vec3));
			binaryio.write(CCAST(&s.extents), sizeof(glm::vec3));
		}

		//int jointOBJNum = sj.jointObjs.size();
		//binaryio.write(CCAST(&jointOBJNum), sizeof(int));
		//for (int i = 0; i < jointOBJNum; ++i) {
		//	binaryio.write(CCAST(&sj.jointObjs[i]), sizeof(JointObject));
		//}
	}

	binaryio.write(CCAST(&s.globalInverseTransform), sizeof(glm::mat4));
	//Insert number of animations
	int numAnim = s.animations.size();
	binaryio.write(CCAST(&numAnim), sizeof(int));
	for (int a = 0; a < numAnim; ++a) {
		//Save skeleton id
		binaryio.write(CCAST(&s.uniqueID), sizeof(uint32_t));
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
				//binaryio.write(CCAST(&s.animations[a].channels[c].keys[k].time), sizeof(float));
				//binaryio.write(CCAST(&aiVec3ToGLM(s.animations[a].channels[c].keys[k].pos.mValue)), sizeof(glm::vec3));
				//binaryio.write(CCAST(&aiQuatToGLM(s.animations[a].channels[c].keys[k].rot.mValue)), sizeof(glm::vec4));
				//binaryio.write(CCAST(&aiVec3ToGLM(s.animations[a].channels[c].keys[k].sca.mValue)), sizeof(glm::vec3));
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
			Face face(&m.vertices);
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



	if (!pScene)
	{
		std::string error = importer.GetErrorString();
		std::cout << "ERROR: " << error << std::endl;
		return false;
	}
	hasAnim = pScene->HasAnimations() && boneVerify(pScene);
	m.name = pScene->GetShortFilename(pFile.c_str());
	//shorten name
	int indexico;
	for (size_t i = 0; i < m.name.size(); ++i) {
		if (m.name.at(i) == '.') {
			indexico = i;
			break;
		}
	}
	m.name = m.name.substr(0, indexico);
	

	if(hasAnim)
	LoadBones(pScene, tds);

	//get data mesh data
	int numtris = 0;
	int numquads = 0;

	auto root = pScene->mRootNode;
	std::map<std::string, aiNode*> sceneChildren;
	for (size_t i = 0; i < root->mNumChildren; ++i) {
		sceneChildren[root->mChildren[i]->mName.data] = root->mChildren[i];
	}

	std::unordered_map<std::string, int> sceneMeshes;
	size_t numMeshes = pScene->mNumMeshes;
	for (size_t i = 0; i < numMeshes; ++i) {
		sceneMeshes[pScene->mMeshes[i]->mName.data] = i;
	}
	for (size_t i = 0; i < pScene->mNumMeshes; ++i) {
		Mesh subset;
		aiMesh* paiMesh = pScene->mMeshes[i];


		//blender does this stupid thing where it chagnes the name from EX: Pants to Pants.001
		//so you have to deprecate the .001 cause they just HAVE to be annoying
		subset.name = paiMesh->mName.C_Str();
		subset.originalName = paiMesh->mName.C_Str();
		for (size_t i = 0; i < subset.name.length(); ++i) {
			if (subset.name[i] == '.')
				subset.name = subset.name.substr(0, i);
		}
		////////////////////Stupid name thing////////////////////

		glm::vec3 maxVert = -glm::vec3(FLT_MAX);
		glm::vec3 minVert = glm::vec3(FLT_MAX);

		//aiMatrix4x4 trans = name_transformMap[paiMesh[i].mName.C_Str()];
		for (size_t v = 0; v < paiMesh->mNumVertices; ++v) {
			aiVector3D vert = paiMesh->mVertices[v];
			aiVector3D norm = aiVector3D();// paiMesh->mNormals[v];
			aiVector3D* txtr = nullptr;
			if (paiMesh->HasTextureCoords(v))
				txtr = paiMesh->mTextureCoords[v];
			if (paiMesh->HasNormals())
				norm = paiMesh->mNormals[v];

			//Transform the verts;
			//auto node = sceneChildren[subset.name];
			//vert *= node->mTransformation.Transpose();

			//subset.vertices.push_back(glm::vec3(paiMesh->mVertices[v].x, paiMesh->mVertices[v].y, paiMesh->mVertices[v].z));
			paiMesh->HasTextureCoords(v) ? subset.vertices.push_back(Vertex(vert, norm, txtr->x, txtr->y)) : subset.vertices.push_back(Vertex(vert, norm));
			maxVert.x = maxVal(maxVert.x, vert.x);
			maxVert.y = maxVal(maxVert.y, vert.y);
			maxVert.z = maxVal(maxVert.z, vert.z);

			minVert.x = minVal(minVert.x, vert.x);
			minVert.y = minVal(minVert.y, vert.y);
			minVert.z = minVal(minVert.z, vert.z);

		}
		for (size_t f = 0; f < paiMesh->mNumFaces; ++f) {
			//subset.faces.push_back(paiMesh->mFaces[f]);
			//TriIndex tri;
			Face face(&subset.vertices);
			for (size_t t = 0; t < paiMesh->mFaces[f].mNumIndices; t++) {
				if (t < 4)
					face.v[t] = paiMesh->mFaces[f].mIndices[t];
			}
			//this is to turn triangles into quads
			int numIndices = paiMesh->mFaces[f].mNumIndices;
			if (numIndices == 3) {
				numtris++;
				face.v[3] = face.v[0];
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


		/////////////////DO THE STUPID NAME THING HERE IF DOESN'T WORK PRIOR/////////////

		//aiMatrix4x4 subchild = sceneChildren.find(subset.name)->second->mTransformation;
		//subset.transform = aiMatrix4x4ToGlm(subchild);
		//m.meshes.push_back(subset);
		ShapeType type = ShapeCheck(subset.name);
		if (type == ShapeType::MESH) { 
			m.meshes.push_back(subset); subset.id = m.meshes.size() - 1; 
		}
		else {
			m.shapes.push_back(ShapeCreate(subset, type));
		}
		///////////////////////////////////////////////////TEST SKELETON TEST SKELETON//////////////////////////////////////////////

			//First check to see if it has bones
		if (paiMesh->mNumBones > 0 && hasAnim) {

			//set up a list of bones and the vertexs for each bone
			std::vector<aiBone*> bones;
			std::vector<std::vector<aiVertexWeight>> weights;
			std::vector<std::vector<int>> boneFaces;

			//load up the data
			for (size_t i = 0; i < paiMesh->mNumBones; ++i) {
				bones.push_back(paiMesh->mBones[i]);


				int boneIndex = tds.nodeIndexes[paiMesh->mBones[i]->mName.data];
				//tds.skeletonJoints[boneIndex].offset = paiMesh->mBones[i]->mOffsetMatrix;
				//tds.skeletonJoints[boneIndex].glOffset = aiMatrix4x4ToGlm(tds.skeletonJoints[boneIndex].invBindPose);
				tds.bones.push_back(paiMesh->mBones[i]);

				//First check if its a shape if o then just 
				if (type == ShapeType::SPHERE) {
					//int index = tds.nodeIndexes[paiMesh->mBones[i]->mName.data];
					tds.skeletonJoints[boneIndex].jointObjs.push_back(JointObject(m.shapes.size() - 1, -1));
				}
				else {
					//This is gonna be a sorted list of vertex weights so
					//use map which is a red-black tree, then do in-order traversal pushback into the vw list
					std::vector<aiVertexWeight> vw;
					std::map<int, aiVertexWeight> vwmap;
					for (size_t b = 0; b < paiMesh->mBones[i]->mNumWeights; b++) {
						auto weight = paiMesh->mBones[i]->mWeights[b];
						vwmap.insert(std::pair<int, aiVertexWeight>(weight.mVertexId, weight));
					}
					for (auto itr : vwmap){// .begin(); itr != vwmap.end(); ++itr) {
						//if(itr.second.mWeight > 0.9f)
						vw.push_back(itr.second);
					}
					weights.push_back(vw);

					//Now that you have a list of in-order vertex id's you can compare with the faces in an efficient manner
					//for each face, check if the bone has f[0] f[1] f[2] and f[3]. it's already sorted so you can iterate through and look for a > value to cancel out
					std::vector<int> bfs;
					for (size_t fid = 0; fid < subset.faces.size(); ++fid) {
						Face* itrFace = &subset.faces[fid];
						bool allfour = true;
						for (size_t i = 0; i < 4; ++i) {
							bool vertFound = true;
							if (vwmap.find(itrFace->v[i]) == vwmap.end()) {
								vertFound = false;
								allfour = false;
								break;
							}
						}
						if (allfour) {
							tds.skeletonJoints[boneIndex].jointObjs.push_back(JointObject(fid, subset.id));
						}
						//bfs.push_back(fid);
					}// auto itrface subset faces begin etc...
				}// if shape or nah
			}//for each bone
			//So here should be where all the bones should be collected, you can compare the bone naems with the tds list and add stuff
		}
	}
	if (hasAnim){// && meshType == SKINNED_MESH) {
		aiMatrix4x4 glit = tds.skeletonJoints[0].transform;
		glit.Inverse();
		s.globalInverseTransform = aiMatrix4x4ToGlm(glit);
		s.joints = tds.skeletonJoints;
		s.numJoints = tds.skeletonJoints.size();
		size_t a = 0;
		for (; a < pScene->mNumAnimations; ++a) {
			aiAnimation* anim = pScene->mAnimations[a];
			PrincipiaAnimation animation;
			animation.name = anim->mName.data;
			animation.duration = anim->mDuration;
			animation.fps = anim->mTicksPerSecond;
			animation.skeletonID = s.uniqueID;

			for (size_t j = 0; j < s.joints.size(); ++j) {
				for (size_t ch = 0; ch < anim->mNumChannels; ch++) {
					aiNodeAnim* ana = anim->mChannels[ch];
					if (!::strcmp(ana->mNodeName.data, s.joints[j].name.c_str())) {
						AnimChannel channel;
						channel.name = ana->mNodeName.data;// anim->mName.data;
						if ((ana->mNumPositionKeys != ana->mNumRotationKeys) || (ana->mNumPositionKeys != ana->mNumScalingKeys) || (ana->mNumRotationKeys != ana->mNumScalingKeys))
							std::cout << "ERROR, UNEVEN KEYS\n";

						for (size_t k = 0; k < ana->mNumRotationKeys; k++) {
							KeySQT key;
							key.pos = ana->mPositionKeys[k];
							key.rot = ana->mRotationKeys[k];
							key.sca = ana->mScalingKeys[k];
							key.time = ana->mRotationKeys[k].mTime;
							channel.keys.push_back(key);
						}
						channel.numKeys = channel.keys.size();
						animation.channels.push_back(channel);
					}
					animation.numChannels = animation.channels.size();
				}
			}
			s.animations.push_back(animation);
		}
	}
	s.name = m.name + "_skel";// tds.skelename;

	for (size_t i = 0; i < m.meshes.size(); ++i)
		m.meshes[i].id = sceneMeshes[m.meshes[i].originalName];
	
	// We're done. Everything will be cleaned up by the importer destructor
	aiNode* rooot = pScene->mRootNode;
	std::vector<std::string> names;
	std::vector<aiMatrix4x4> transforms;
	for (size_t i = 0; i < rooot->mNumChildren; ++i) {
		transforms.push_back(rooot->mChildren[i]->mTransformation);
		names.push_back(rooot->mChildren[i]->mName.C_Str());
	}
	return true;
}

bool LoadDirectory(std::string directory, bool triangulate)
{
	for (const auto & p : fs::directory_iterator(directory)) {
		UID++;
		PrincipiaModel mod;
		PrincipiaSkeleton skeleton;
		mod.uniqueID = newUniqueID(p.path().stem().string());
		skeleton.uniqueID = newUniqueID(p.path().stem().string() + "_skel");
		mod.skeletonID = skeleton.uniqueID;
		if (DoTheImportThing(p.path().string(), mod, skeleton, triangulate)) {
			//break;


			if (hasAnim) {
				ConvertJointVerts(mod, skeleton);
				GetJointExtents(mod, skeleton);
				//ModelScaler(mod);
				//SkeletonScaler(skeleton);
			}
			//else
			ModelScaler(mod);

			//Build the bvh
			Build(mod);

			if (triangulate)
				mod.name += "_t";
			WritePEModel(mod,"Output/"+ mod.name + ".pm");
			if(hasAnim) 
				WriteSkeleton(skeleton, "Output/" + mod.name + ".pa");
		}
	}
	return false;
}


#pragma region skeletonbiulding

bool LoadBones(const aiScene* scene, tempDataStruct& tds) {
	std::vector<aiNode*> nodes;

	auto armature = scene->mRootNode->mChildren[0];
	auto numSceneChildren = armature->mNumChildren;

	/*Takes the Root animation node and Bredth-First-Searches the tree to have a flat,level-order array of the animation*/
	std::queue<aiNode*> nodeQueue;
	nodeQueue.push(armature);
	while (!nodeQueue.empty()) {
		size_t qsize = nodeQueue.size();
		for (size_t i = 0; i < qsize; ++i) {
			aiNode* n = nodeQueue.front();
			nodeQueue.pop();
			nodes.push_back(n);
			for (size_t c = 0; c < n->mNumChildren; c++)
				nodeQueue.push(n->mChildren[c]);
		}
	}

	//LEGACY ALERT
	//This gets a hashmap of the names of everybone the animations use
	std::unordered_map<std::string, bool> boneNameMap;
	auto numAnims = scene->mNumAnimations;
	for (size_t i = 0; i < numAnims; ++i) {
		auto numChannels = scene->mAnimations[i]->mNumChannels;
		for (size_t c = 0; c < numChannels; c++) {
			boneNameMap[scene->mAnimations[i]->mChannels[c]->mNodeName.data] = true;
		}
	}

	//This gets a hashmap of the names of everybone the meshs use
	//std::unordered_map<std::string, bool> boneNameMap;
	//std::unordered_map<std::string, aiBone*> boneMap;
	//for (size_t i = 0; i < scene->mNumMeshes; i++) {
	//	aiMesh* mesh = scene->mMeshes[i];
	//	for (size_t c = 0; c < mesh->mNumBones; c++) {
	//		boneNameMap[mesh->mBones[c]->mName.data] = true;
	//		boneMap[mesh->mBones[c]->mName.data] = mesh->mBones[c];
	//	}
	//}

	//So we have a flat list of the rootnames, and a hash of the anim names
	//we now go through the list of rootnames and if its in the anim names, push it in a new array
	std::vector<aiNode*> confirmedNodes;
	confirmedNodes.reserve(boneNameMap.size());
	for (auto i : nodes) {
		if (boneNameMap[i->mName.data] == true)
			confirmedNodes.emplace_back(i);
	}

	//So now we need the indexes of the parents that each confirmed node has
	//create a hashmap of the node with their indexes and then a list that compares parentnames with indexes
	std::unordered_map<std::string, int> confirmedNodeIndexes;
	auto numConfirmedNodes = confirmedNodes.size();
	for (size_t i = 0; i < numConfirmedNodes; ++i) {
		confirmedNodeIndexes[confirmedNodes[i]->mName.data] = i;
	}

	//list starts at one because rootnode wont be there
	std::vector<int> parentIndexes;
	parentIndexes.reserve(numConfirmedNodes);
	parentIndexes.emplace_back(-1);
	for (size_t i = 1; i < numConfirmedNodes; ++i) {
		parentIndexes.emplace_back(confirmedNodeIndexes[confirmedNodes[i]->mParent->mName.data]);
	}

	//Now we load up the joints
	tds.skeletonJoints.reserve(numConfirmedNodes);
	for (size_t i = 0; i < numConfirmedNodes; ++i) {
		Joint j;
		j.name = confirmedNodes[i]->mName.data;
		j.parentIndex = parentIndexes[i];
		j.transform = confirmedNodes[i]->mTransformation;
		j.invBindPose = confirmedNodes[i]->mTransformation.Inverse();// Transpose();
		j.glTransform = aiMatrix4x4ToGlm(j.transform);
		j.glInvBindPose = aiMatrix4x4ToGlm(j.invBindPose);
		//j.offset = boneMap[j.name]->mOffsetMatrix;
		//j.glOffset = aiMatrix4x4ToGlm(j.offset);

		tds.skeletonJoints.emplace_back(j);
	}
	//Everything should technically be set up by now 
	//EXCEPT for the fact that the invBindPose is probably wrong and needs to be based off the bone itself and nto just the node
	//So we now need to get the bone from the mesh since for some ridiculous reason they didn't leave this info in the node
	//actually one thing we can do is just manually make these all by combinding the transforms
	//or maybe just output all the relevant data uch as...
	/*
	1. confirmedNodeIndexes
	2. parentindexes
	3. confirmedNodeIndexes;
	*/
	//Then construct the data on the meshes/bones import
	tds.confirmedNodes = std::move(confirmedNodes);
	tds.parentIndexes = std::move(parentIndexes);
	tds.nodeIndexes = std::move(confirmedNodeIndexes);

	return true;
}

#pragma endregion
