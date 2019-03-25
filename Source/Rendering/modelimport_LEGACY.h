#pragma once
#ifndef MODELIMPORT_H
#define MODELIMPORT_H

#include "rendermanagers.h"



struct TempModelData
{
	int NumVerts;						//Numver of Verts in model
	int NumTris;						//Number of Triangles in model

	std::vector<Vertex> Vertexes;
	std::vector<int> Indices;         // Models index list

	AABB AABox;

	glm::vec3 Center;						// True center of the model

	std::vector<glm::vec3> tempAABB;        // Stores models AABB (min vertex, max vertex, and center)
											// Where AABB[0] is the min Vertex, and AABB[1] is the max vertex
	float BoundingSphere;							// Model's bounding sphere
};

class ModelImport
{
public:
	ModelImport();
	~ModelImport();

	bool LoadTEModel(std::vector<TriangleVert>& vertz, std::vector<TriangleIndex>& indz, std::vector<Mesh>& meshes, int& currID, std::string fileName);
private:
	int currInd;
	VkResult LoadBuffers(VulkanDevice& device, StaticModel& mod, const TempModelData& tempMod);
	VkResult LoadAABBuffers(VulkanDevice& device, StaticModel& mod, const TempModelData& tempMod);
	AABB createAABB(int start, int end, const std::vector<TriangleVert>& vertz, const std::vector<TriangleIndex>& indz);
};
#endif // !MODELIMPORT_H