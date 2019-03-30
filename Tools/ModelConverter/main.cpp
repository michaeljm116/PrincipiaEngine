#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <filesystem>
#include <array>
#include <set>
#include <map>
#include <chrono>


namespace fs = std::filesystem;

#ifndef maxVal
#define maxVal(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef minVal
#define minVal(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define CCAST reinterpret_cast<char*>


//Found from: https://stackoverflow.com/questions/29184311/how-to-rotate-a-skinned-models-bones-in-c-using-assimp
//inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
//{
//	glm::mat4 to;
//
//	to[0][0] = (glm::float32)from->a1; to[0][1] = (glm::float32)from->b1;  to[0][2] = (glm::float32)from->c1; to[0][3] = (glm::float32)from->d1;
//	to[1][0] = (glm::float32)from->a2; to[1][1] = (glm::float32)from->b2;  to[1][2] = (glm::float32)from->c2; to[1][3] = (glm::float32)from->d2;
//	to[2][0] = (glm::float32)from->a3; to[2][1] = (glm::float32)from->b3;  to[2][2] = (glm::float32)from->c3; to[2][3] = (glm::float32)from->d3;
//	to[3][0] = (glm::float32)from->a4; to[3][1] = (glm::float32)from->b4;  to[3][2] = (glm::float32)from->c4; to[3][3] = (glm::float32)from->d4;
//
//	return to;
//}

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2
#define W_AXIS 3

glm::mat4 axisChange(const glm::mat4& from) {
	glm::mat4 to;

	to[0][X_AXIS] = from[0][X_AXIS];
	for (int i = 0; i < 4; ++i) {
		to[i][X_AXIS] = from[i][X_AXIS];
		to[i][Y_AXIS] = from[i][Y_AXIS];
		to[i][Z_AXIS] = from[i][Z_AXIS];
		to[i][W_AXIS] = from[i][W_AXIS];
	}

	return to;
}

glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from)
{
	glm::mat4 to;

	to[0][0] = (glm::float32)from.a1;	to[1][0] = (glm::float32)from.b1;	to[2][0] = (glm::float32)from.c1;	to[3][0] = (glm::float32)from.d1;
	to[0][1] = (glm::float32)from.a2;	to[1][1] = (glm::float32)from.b2;	to[2][1] = (glm::float32)from.c2;	to[3][1] = (glm::float32)from.d2;
	to[0][2] = (glm::float32)from.a3;	to[1][2] = (glm::float32)from.b3;	to[2][2] = (glm::float32)from.c3;	to[3][2] = (glm::float32)from.d3;
	to[0][3] = (glm::float32)from.a4;	to[1][3] = (glm::float32)from.b4;	to[2][3] = (glm::float32)from.c4;	to[3][3] = (glm::float32)from.d4;

	return to;
	//return axisChange(to);
}

glm::vec4 aiQuatToGLM(const aiQuaternion& from) {
	glm::vec4 to;
	to.x = from.x;
	to.y = from.y;
	to.z = from.z;
	to.w = from.w;
	return to;
}

#define MAX_BONES 64
// Maximum number of bones per vertex
#define MAX_BONES_PER_VERTEX 4

// Skinned mesh class

// Per-vertex bone IDs and weights
struct VertexBoneData
{
	std::array<uint32_t, MAX_BONES_PER_VERTEX> IDs;
	std::array<float, MAX_BONES_PER_VERTEX> weights;

	// Ad bone weighting to vertex info
	void add(uint32_t boneID, float weight)
	{
		for (uint32_t i = 0; i < MAX_BONES_PER_VERTEX; i++)
		{
			if (weights[i] == 0.0f)
			{
				IDs[i] = boneID;
				weights[i] = weight;
				return;
			}
			
		}
	}
	void average() {
		float total = 0;
		for (int i = 0; i < MAX_BONES_PER_VERTEX; ++i) {
			total += weights[i];
		}
		assert(total != 0);
		for (int i = 0; i < MAX_BONES_PER_VERTEX;) {
			weights[i] /= total;
		}
	}
};

struct TriIndex {
	int v[3];
};
struct Vertex {
	glm::vec3 position;
	Vertex() {};
	Vertex(glm::vec3 p) { position = p; };
};
struct Mesh {
	std::string name;
	std::vector<TriIndex> tris;
	std::vector<Vertex> vertices;
	std::vector<VertexBoneData> bones;
	glm::vec3 extent;
	glm::vec3 center;
};

struct  Joint
{
	std::string name;
	int parentIndex;
	aiMatrix4x4 invBindPose;
	aiMatrix4x4 transform;
	glm::mat4 glInvBindPose;
	glm::mat4 glTransform;
};

//how bout 3 vec3's a time and a trigger  or 2?
struct KeySQT {
	aiVectorKey pos;
	aiQuatKey rot;
	aiVectorKey sca;
	float time;
};

struct AnimChannel {
	std::string name;
	int numKeys;
	std::vector<KeySQT> keys;
};

struct PrincipiaAnimation {
	int skeletonID;
	int numChannels;
	std::string name;
	std::vector<AnimChannel> channels;
	float duration;
	float fps;
};

struct  PrincipiaSkeleton
{
	int uniqueID;
	int numJoints;
	std::vector<Joint> joints;
	std::string name;
	glm::mat4 globalInverseTransform;
	std::vector<PrincipiaAnimation> animations;

	glm::vec3 center;
	glm::vec3 extents;
	
};

struct PrincipiaModel {
	std::string name;
	std::vector<Mesh> meshes;
	int uniqueID;
	int skeletonID;
	glm::vec3 center;
	glm::vec3 extents;
};

struct RJoint{
	std::vector<TriIndex> tris;
	std::vector<int> shapes;
	std::string name;
	int parentIndex;


	aiMatrix4x4 invBindPose;
	aiMatrix4x4 transform;
	glm::mat4 glInvBindPose;
	glm::mat4 glTransform;

	glm::vec3 center;
	glm::vec3 extents;
};
struct PrincipiaSkinnedModel {
	int uniqueID;
	int skeletonID;
	std::vector<Vertex> verts;
	std::vector<RJoint> joints;
};

struct tempRJoint {
	std::string name;
	std::vector<int> verts;
};


int UID = 0;
int newUniqueID() {
	UID++;	
	int time = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
	return time + UID;
};
auto cmp = [](std::pair<std::string, int> const & a, std::pair<std::string, int> const & b)
{
	return a.second != b.second ? a.second < b.second : a.first < b.first;
};

bool DoTheImportThing(const std::string& pFile, PrincipiaModel& m, PrincipiaSkeleton& s);

bool WritePEModel(PrincipiaModel& m, std::string fn);
bool WriteSkeleton(PrincipiaSkeleton& s, std::string fn);


PrincipiaModel ReadPEModel(const char* pFile);
bool ModelScaler(PrincipiaModel& m);
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
		int numTris = m.meshes[i].tris.size();

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
			binaryio.write(CCAST(&m.meshes[i].vertices[v]), sizeof(glm::vec3));
		}
		for (int t = 0; t < numTris; t++) {
			binaryio.write(CCAST(&m.meshes[i].tris[t]), sizeof(TriIndex));
		}

		if (skinned) {
			int numBones = m.meshes[i].bones.size();
			binaryio.write(CCAST(&numBones), sizeof(int));
			for (int b = 0; b < numBones; ++b) {
				binaryio.write(CCAST(&m.meshes[i].bones[b].IDs), sizeof(int) * 4);
				binaryio.write(CCAST(&m.meshes[i].bones[b].weights), sizeof(float) * 4);
			}
		}

	}

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
		int numTris;

		//name
		binaryio.read(CCAST(&meshNameLength), sizeof(int));
		for (int n = 0; n < meshNameLength; ++n) {
			binaryio.read(&c, sizeof(char));
			m.name.push_back(c);
		}
		//nums
		binaryio.read(CCAST(&numVerts), sizeof(int));
		binaryio.read(CCAST(&numTris), sizeof(int));

		//aabbs
		binaryio.read(CCAST(&m.center), sizeof(glm::vec3));
		binaryio.read(CCAST(&m.extent), sizeof(glm::vec3));

		for (int v = 0; v < numVerts; ++v) {
			glm::vec3 vert;
			binaryio.read(CCAST(&vert), sizeof(glm::vec3));
			m.vertices.push_back(vert);
		}
		for (int t = 0; t < numTris; ++t) {
			TriIndex tri;
			binaryio.read(CCAST(&tri), sizeof(TriIndex));
			m.tris.push_back(tri);
		}
		mod.meshes.push_back(m);
	}

	return mod;
}
bool DoTheImportThing(const std::string& pFile, PrincipiaModel& m, PrincipiaSkeleton& s)
{
	tempDataStruct tds;
	// Create an instance of the Importer class
	Assimp::Importer importer;
	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* pScene = importer.ReadFile(pFile,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);
	// If the import failed, report it
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
	

	LoadBones(pScene, tds);

	//get data mesh data
	for (int i = 0; i < pScene->mNumMeshes; ++i) {
		Mesh subset;
		glm::vec3 maxVert = glm::vec3(FLT_MIN);
		glm::vec3 minVert = glm::vec3(FLT_MAX);

		aiMesh* paiMesh = pScene->mMeshes[i];
		for (int v = 0; v < paiMesh->mNumVertices; ++v) {
			subset.vertices.push_back(glm::vec3(paiMesh->mVertices[v].x, paiMesh->mVertices[v].y, paiMesh->mVertices[v].z));
			maxVert.x = maxVal(maxVert.x, paiMesh->mVertices[v].x);
			maxVert.y = maxVal(maxVert.y, paiMesh->mVertices[v].y);
			maxVert.z = maxVal(maxVert.z, paiMesh->mVertices[v].z);

			minVert.x = minVal(minVert.x, paiMesh->mVertices[v].x);
			minVert.y = minVal(minVert.y, paiMesh->mVertices[v].y);
			minVert.z = minVal(minVert.z, paiMesh->mVertices[v].z);		

		}
		for (int f = 0; f < paiMesh->mNumFaces; ++f) {
			//subset.faces.push_back(paiMesh->mFaces[f]);
			TriIndex tri;
			for (int t = 0; t < 3; t++)
				tri.v[t] = paiMesh->mFaces[f].mIndices[t];
			subset.tris.push_back(tri);
		}

		subset.center.x = (maxVert.x + minVert.x) * 0.5f;
		subset.center.y = (maxVert.y + minVert.y) * 0.5f;
		subset.center.z = (maxVert.z + minVert.z) * 0.5f;

		subset.extent.x = (maxVert.x - minVert.x) * 0.5f;
		subset.extent.y = (maxVert.y - minVert.y) * 0.5f;
		subset.extent.z = (maxVert.z - minVert.z) * 0.5f;

		subset.name = paiMesh->mName.C_Str();
		m.meshes.push_back(subset);

	/////////////////////////////////////////////////TEST SKELETON TEST SKELETON//////////////////////////////////////////////
	/////////////////////////////////////////////////TEST SKELETON TEST SKELETON//////////////////////////////////////////////
	/////////////////////////////////////////////////TEST SKELETON TEST SKELETON//////////////////////////////////////////////
		std::vector<tempRJoint> rayJoints;
		if (paiMesh->mNumBones > 0) {
			std::vector<VertexBoneData> bonesy;
			m.meshes[i].bones.resize(paiMesh->mNumVertices);

			for (int b = 0; b < paiMesh->mNumBones; ++b) {
				aiBone* boney = paiMesh->mBones[b];
				//std::vector<aiVertexWeight> weights;
				tempRJoint rj;
				for (int w = 0; w < boney->mNumWeights; w++) {
					aiVertexWeight weighty = boney->mWeights[w];
					//m.meshes[i].bones[weighty.mVertexId].add(tds.jointMap.find(boney->mName.data)->second, weighty.mWeight);
					//weights.push_back(weighty);
						if (weighty.mWeight > 0.5f){
							rj.verts.push_back(weighty.mVertexId);
					}
				}
				rj.name = boney->mName.C_Str();
				rayJoints.push_back(rj);				
				//skelly.joints.push_back(jointy);
			}
			int total = 0;
			for (int i = 0; i < rayJoints.size(); ++i)
				total += rayJoints[i].verts.size();
			int a = 0;
			//m.meshes[i].v

			//skellys.push_back(skelly);
			/*for (int f = 0; f < paiMesh->mNumVertices; ++f) {
				m.meshes[i].vertices[f].boneData = bonesy[f];
			}*/
		}
	}

	if (pScene->HasAnimations() && meshType == SKINNED_MESH) {
		meshType = SKIN_AND_ANIM;
		aiMatrix4x4 glit = pScene->mRootNode->mTransformation;
		glit.Inverse();
		s.globalInverseTransform = aiMatrix4x4ToGlm(glit);
		s.joints = tds.skeletonJoints;
		s.numJoints = tds.skeletonJoints.size();
		int a = 0;
		for (; a < pScene->mNumAnimations; ++a) {
			aiAnimation* anim = pScene->mAnimations[a];
			PrincipiaAnimation animation;
			animation.name = anim->mName.data;
			animation.duration = anim->mDuration;
			animation.fps = anim->mTicksPerSecond;
			animation.skeletonID = s.uniqueID;

			for (int j = 0; j < s.joints.size(); ++j) {
				for (int ch = 0; ch < anim->mNumChannels; ch++) {
					aiNodeAnim* ana = anim->mChannels[ch];
					if (!::strcmp(ana->mNodeName.data, s.joints[j].name.c_str())) {
						AnimChannel channel;
						channel.name = ana->mNodeName.data;// anim->mName.data;
						if ((ana->mNumPositionKeys != ana->mNumRotationKeys) || (ana->mNumPositionKeys != ana->mNumScalingKeys) || (ana->mNumRotationKeys != ana->mNumScalingKeys))
							std::cout << "ERROR, UNEVEN KEYS\n";

						for (int k = 0; k < ana->mNumRotationKeys; k++) {
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
	s.name = tds.skelename;
	// We're done. Everything will be cleaned up by the importer destructor
	return true;
}

bool ModelScaler(PrincipiaModel& m) {
	glm::mat4 world = glm::mat4(1);
	float maxE = FLT_MIN;
	for (int i = 0; i < m.meshes.size(); ++i) {
		for (int j = 0; j < 3; ++j) {
			maxE = maxVal(m.meshes[i].extent[j], maxE);
		}
	}

	//compare it to size
	float ratio = 1 / maxE;
	
	//Scale it
	world = glm::scale(world, glm::vec3(ratio));

	//Scale the bounds
	for (int i = 0; i < m.meshes.size(); ++i) {
		m.meshes[i].center = glm::vec3(world * glm::vec4(m.meshes[i].center, 1.f));
		m.meshes[i].extent = glm::vec3(world * glm::vec4(m.meshes[i].extent, 1.f));
		for (int j = 0; j < m.meshes[i].vertices.size(); j++) {
			m.meshes[i].vertices[j] = glm::vec3(world * glm::vec4(m.meshes[i].vertices[j].position, 1.f));
		}
	}

	//Transform the verts to fit the size

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
		if (DoTheImportThing(p.path().string(), mod, skeleton)) {
			//break;
			ModelScaler(mod);
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
