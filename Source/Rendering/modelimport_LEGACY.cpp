#include "modelimport.h"

//#include <istream>
//#include <sstream>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

ModelImport::ModelImport()
{
}

ModelImport::~ModelImport()
{
}

bool ModelImport::LoadTEModel(std::vector<TriangleVert>& vertz, std::vector<TriangleIndex>& indz, std::vector<Mesh>& meshes, int& currID, std::string fileName) {
	//Temporary structures
	std::vector<int> subsetStarts;
	std::vector<int> subsetMatIDs;
	//float tempVert[3];
	Material tempMaterial;
	//int tempIndex, tempSubIndex;
	glm::vec3 center, extents;
	//Temporarily store the model's data
	TempModelData data;

	//Open the File, if it fails then return false
	std::ifstream binaryio;
	binaryio.open(fileName, std::ios::in | std::ios::binary);
	if (binaryio.fail())
		return false;

	//Read the Ints that display the quantity of the structs
	int numVerts, numIndices, numTriangles, numMaterials; 
	binaryio.read(reinterpret_cast<char*>(&numVerts), sizeof(numVerts));
	binaryio.read(reinterpret_cast<char*>(&numIndices), sizeof(numIndices));
	binaryio.read(reinterpret_cast<char*>(&numTriangles), sizeof(numTriangles));
	binaryio.read(reinterpret_cast<char*>(&numMaterials), sizeof(numMaterials));

	//to decide if you wanna use tangents
	bool useTang; 
	binaryio.read(reinterpret_cast<char*>(&useTang), sizeof(useTang));

	//model.subsets = numMaterials;
	data.NumVerts = numVerts;
	data.NumTris = numTriangles;

	//Read and Load the Verts
	for (int i = 0; i < numVerts; i++)
	{
		float tempPos[3];
		float tempUV[2];
		float tempNorm[3];

		binaryio.read(reinterpret_cast<char*>(&tempPos), sizeof(tempPos));
		binaryio.read(reinterpret_cast<char*>(&tempUV), sizeof(tempUV));
		binaryio.read(reinterpret_cast<char*>(&tempNorm), sizeof(tempNorm));

		Vertex tVert;
		tVert.pos.x = tempPos[0];
		tVert.pos.y = tempPos[1];
		tVert.pos.z = tempPos[2];

		tVert.uv.x = tempUV[0];
		tVert.uv.y = tempUV[1];

		tVert.norm.x = tempNorm[0];
		tVert.norm.y = tempNorm[1];
		tVert.norm.z = tempNorm[2];

		if (useTang)
		{
			float tempTang[3];
			float tempBino[3];
			binaryio.read(reinterpret_cast<char*>(&tempTang), sizeof(tempTang));
			binaryio.read(reinterpret_cast<char*>(&tempBino), sizeof(tempBino));

			tVert.tang.x = tempTang[0];
			tVert.tang.y = tempTang[1];
			tVert.tang.z = tempTang[2];

			//tVert.biTangent.x = tempVert.bino.pos[0];
			//tVert.biTangent.y = tempVert.bino.pos[1];
			//tVert.biTangent.z = tempVert.bino.pos[2];
		}

		data.Vertexes.push_back(tVert);
	}

	//Read and Load The Indices
	for (int i = 0; i < numIndices; i++)
	{
		int tempIndex;
		binaryio.read(reinterpret_cast<char*>(&tempIndex), sizeof(tempIndex));
		data.Indices.push_back(tempIndex);
	}

	//Read the Subset Indices
	for (int i = 0; i < numMaterials; i++)
	{
		int tempSubIndex;
		binaryio.read(reinterpret_cast<char*>(&tempSubIndex), sizeof(tempSubIndex));
		//model.subsetIndexStart.push_back(tempSubIndex);
		subsetStarts.push_back(tempSubIndex);
	}
	//model.subsetIndexStart.push_back(numIndices);


	//Read the materials
	for (int i = 0; i < numMaterials; i++)
	{
		//-+MyMaterial mat;

		int matID;
		std::string matName;

		std::string diffTex;
		std::string specTex;
		std::string normMap;

		int matNameLen;
		binaryio.read(reinterpret_cast<char*>(&matNameLen), sizeof(int));
		for (int i = 0; i < matNameLen; i++) {
			char c;
			binaryio.read(&c, sizeof(c));
			matName.push_back(c);
		}
		binaryio.read(reinterpret_cast<char*>(&matID), sizeof(int));

		float ambient[3], diffuse[3], specular[3];
		binaryio.read(reinterpret_cast<char*>(&ambient), sizeof(ambient));
		binaryio.read(reinterpret_cast<char*>(&diffuse), sizeof(diffuse));
		binaryio.read(reinterpret_cast<char*>(&specular), sizeof(specular));

		bool hasDiffTexture, hasSpecTexture, hasNormMap;
		binaryio.read(reinterpret_cast<char*>(&hasDiffTexture), sizeof(bool));
		binaryio.read(reinterpret_cast<char*>(&hasSpecTexture), sizeof(bool));
		binaryio.read(reinterpret_cast<char*>(&hasNormMap), sizeof(bool));

		int diffTexLen, specTexLen, normMapLen;	  //Length of the material name and texture names
		binaryio.read(reinterpret_cast<char*>(&diffTexLen), sizeof(int));
		binaryio.read(reinterpret_cast<char*>(&specTexLen), sizeof(int));
		binaryio.read(reinterpret_cast<char*>(&normMapLen), sizeof(int));

		for (int i = 0; i < diffTexLen; i++) {
			char c;
			binaryio.read(&c, sizeof(c));
			diffTex.push_back(c);
		}
		for (int i = 0; i < specTexLen; i++) {
			char c;
			binaryio.read(&c, sizeof(c));
			specTex.push_back(c);
		}
		for (int i = 0; i < normMapLen; i++) {
			char c;
			binaryio.read(&c, sizeof(c));
			normMap.push_back(c);
		}

		//Load into Surface Material
		Material tempSMat;
		tempSMat.name = matName;
		tempSMat.params.ambient = glm::vec4(ambient[0], ambient[1], ambient[2], 1);
		tempSMat.params.diffuse = glm::vec4(diffuse[0], diffuse[1], diffuse[2], 1);
		tempSMat.params.specular = glm::vec4(specular[0], specular[1], specular[2], 1);

		tempSMat.hasDiffTexture = hasDiffTexture;
		tempSMat.hasSpecTexture = hasSpecTexture;
		tempSMat.hasNormTexture = hasNormMap;

#pragma region Load Textures
		//For Diffuse Textures....
		if (hasDiffTexture)
		{
			bool copy = false;
			for (size_t i = 0; i < TEXTUREMANAGER->diffuseTextureNameArray.size(); i++)
			{
				if (diffTex == TEXTUREMANAGER->diffuseTextureNameArray[i])
				{
					copy = true;
					tempSMat.diffTextureID = i;
				}
			}
			if (!copy)
			{
				TEXTUREMANAGER->diffuseTextureNameArray.push_back(diffTex);
				tempSMat.diffTextureID = TEXTUREMANAGER->diffuseTextureNameArray.size() - 1;
			}
		}

		//For Specular Textures...
		if (hasSpecTexture)
		{
			bool copy = false;
			for (size_t i = 0; i < TEXTUREMANAGER->specularTextureNameArray.size(); i++)
			{
				if (specTex == TEXTUREMANAGER->specularTextureNameArray[i])
				{
					copy = true;
					tempSMat.specTextureID = i;
				}
			}
			if (!copy)
			{
				TEXTUREMANAGER->specularTextureNameArray.push_back(specTex);
				tempSMat.specTextureID = TEXTUREMANAGER->specularTextureNameArray.size() - 1;
			}
		}

		//For Normal Maps...
		if (hasNormMap)
		{
			bool copy = false;
			for (size_t i = 0; i < TEXTUREMANAGER->normalTextureNameArray.size(); i++)
			{
				if (normMap  == TEXTUREMANAGER->normalTextureNameArray[i])
				{
					copy = true;
					tempSMat.normTextureID = i;
				}
			}
			if (!copy)
			{
				TEXTUREMANAGER->normalTextureNameArray.push_back(normMap);
				tempSMat.normTextureID = TEXTUREMANAGER->normalTextureNameArray.size() - 1;
			}
		}
#pragma endregion this loads the textures and compares with the texture manager

		// Check to see if the material is alreayd there, if not then put it in the array
		bool copy = false;
		for (size_t i = 0; i < MATERIALMANAGER->materials.size(); ++i)
		{
			if (tempSMat.name == MATERIALMANAGER->materials[i].name)
			{
				copy = true;
				//model.subsetMaterialID.push_back(i);
				subsetMatIDs.push_back(i);
			}
		}
		if (!copy)
		{
			MATERIALMANAGER->materials.push_back(tempSMat);
			//model.subsetMaterialID.push_back(MATERIALMANAGER->materials.size() - 1);
			subsetMatIDs.push_back(MATERIALMANAGER->materials.size() - 1);
		}
	}

	binaryio.read(reinterpret_cast<char*>(&center), sizeof(center));
	binaryio.read(reinterpret_cast<char*>(&extents), sizeof(extents));

	data.AABox.center.x = center.x;
	data.AABox.center.y = center.y;
	data.AABox.center.z = center.z;

	data.AABox.extents.x = extents.x;
	data.AABox.extents.y = extents.y;
	data.AABox.extents.z = extents.z;

	// Load the Buffers
	//VK_CHECKRESULT(LoadBuffers(device, model, data), "LOAD BUFFERS");
	//VK_CHECKRESULT(LoadAABBuffers(device, model, data), "LOAD AABB BUFFERS");
	int prevSize = indz.size();
	int prevSizeVert = vertz.size();
	for (int i = 0; i < data.NumVerts; ++i) {
		vertz.push_back(TriangleVert(data.Vertexes[i].pos));
	}
	for (size_t i = 0; i < data.Indices.size(); i += 3) {
		currID++;
		indz.push_back(TriangleIndex(data.Indices[i]+ prevSizeVert, data.Indices[i + 1]+ prevSizeVert, data.Indices[i + 2] + prevSizeVert, currID));
	}

	for (int i = 0; i < numMaterials; ++i) {
		currID++;
		Mesh mesh;
		mesh.diffuse = MATERIALMANAGER->materials[subsetMatIDs[i]].params.diffuse;
		mesh.specular = 32.0f;
		mesh.id = currID;
		mesh.startIndex = (subsetStarts[i] / 3) + prevSize;
		if (i < numMaterials - 1)
			mesh.endIndex = (subsetStarts[i + 1] / 3) + prevSize - 1;
		else
			mesh.endIndex = indz.size() - 1;

		//Calculate the extents; this is so wasteful but you can optimize dis jenkz later
		AABB aabb = createAABB(mesh.startIndex, mesh.endIndex, vertz, indz);
		mesh.center = aabb.center;
		mesh.extents = aabb.extents;

		meshes.push_back(mesh);
	}

	return true;
}

VkResult ModelImport::LoadBuffers(VulkanDevice& device, StaticModel& mod, const TempModelData& tempMod)
{
	//VERTEX BUFFER
	VkDeviceSize bufferSize = sizeof(tempMod.Vertexes[0]) * tempMod.Vertexes.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, tempMod.Vertexes.data(), (size_t)bufferSize);
	vkUnmapMemory(device.logicalDevice, stagingBufferMemory);

	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mod.vertBuff, mod.vertBuffMemory);

	device.copyBuffer(stagingBuffer, mod.vertBuff, bufferSize);
	vkDestroyBuffer(device.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(device.logicalDevice, stagingBufferMemory, nullptr);

	// INDEX BUFFER
	VkBuffer stagingIndexBuffer;
	VkDeviceMemory stagingIndexBufferMemory;
	bufferSize = sizeof(tempMod.Indices[0]) * tempMod.Indices.size();
	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexBufferMemory);

	vkMapMemory(device.logicalDevice, stagingIndexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, tempMod.Indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device.logicalDevice, stagingIndexBufferMemory);

	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mod.indexBuff, mod.indexBuffMemory);

	device.copyBuffer(stagingIndexBuffer, mod.indexBuff, bufferSize);

	vkDestroyBuffer(device.logicalDevice, stagingIndexBuffer, nullptr);
	vkFreeMemory(device.logicalDevice, stagingIndexBufferMemory, nullptr);
	
	return VK_SUCCESS;
}

VkResult ModelImport::LoadAABBuffers(VulkanDevice& device, StaticModel& mod, const TempModelData& tempMod)
{
	//------------------------------------------------//
	//-------------------AABB-------------------------//
	//------------------------------------------------//
	
	glm::vec3 minVertex;
	glm::vec3 maxVertex;

	maxVertex.x = tempMod.AABox.center.x + tempMod.AABox.extents.x;
	maxVertex.y = tempMod.AABox.center.y + tempMod.AABox.extents.y;
	maxVertex.z = tempMod.AABox.center.z + tempMod.AABox.extents.z;

	minVertex.x = tempMod.AABox.center.x - tempMod.AABox.extents.x;
	minVertex.y = tempMod.AABox.center.y - tempMod.AABox.extents.y;
	minVertex.z = tempMod.AABox.center.z - tempMod.AABox.extents.z;

	//Build Vertices for AABB
	ShapeVertex verticesAABB[] =
	{
		{ minVertex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
	{ glm::vec3(minVertex.x, maxVertex.y, minVertex.z), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
	{ glm::vec3(maxVertex.x, maxVertex.y, minVertex.z), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
	{ glm::vec3(maxVertex.x, minVertex.y, minVertex.z), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
	{ glm::vec3(minVertex.x, minVertex.y, maxVertex.z), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
	{ glm::vec3(minVertex.x, maxVertex.y, maxVertex.z), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
	{ maxVertex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) },
	{ glm::vec3(maxVertex.x, minVertex.y, maxVertex.z), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) }
	};

	//Build Indices for AABB
	uint32_t indicesAABB[] =
	{
		// front face
		0, 1, 2, 0, 2, 3,
		// back face
		4, 6, 5, 4, 7, 6,
		// left face
		4, 5, 1, 4, 1, 0,
		// right face
		3, 2, 6, 3, 6, 7,
		// top face
		1, 5, 6, 1, 6, 2,
		// bottom face
		4, 0, 3, 4, 3, 7
	};

	//VERTEX BUFFER
	VkDeviceSize bufferSize = sizeof(verticesAABB[0]) * 8;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, verticesAABB, (size_t)bufferSize);
	vkUnmapMemory(device.logicalDevice, stagingBufferMemory);

	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mod.AABBVertBuff, mod.AABBVertMemory);

	device.copyBuffer(stagingBuffer, mod.AABBVertBuff, bufferSize);
	vkDestroyBuffer(device.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(device.logicalDevice, stagingBufferMemory, nullptr);

	// INDEX BUFFER
	VkBuffer stagingIndexBuffer;
	VkDeviceMemory stagingIndexBufferMemory;
	bufferSize = sizeof(indicesAABB[0]) * 36;
	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIndexBuffer, stagingIndexBufferMemory);

	vkMapMemory(device.logicalDevice, stagingIndexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indicesAABB, (size_t)bufferSize);
	vkUnmapMemory(device.logicalDevice, stagingIndexBufferMemory);

	device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mod.AABBIndexBuff, mod.AABBIndexMemory);

	device.copyBuffer(stagingIndexBuffer, mod.AABBIndexBuff, bufferSize);

	vkDestroyBuffer(device.logicalDevice, stagingIndexBuffer, nullptr);
	vkFreeMemory(device.logicalDevice, stagingIndexBufferMemory, nullptr);

	return VK_SUCCESS;
}

AABB ModelImport::createAABB(int start, int end, const std::vector<TriangleVert>& vertz, const std::vector<TriangleIndex>& indz)
{
	AABB aabb;
	glm::vec3 minVertex;
	glm::vec3 maxVertex;

	minVertex[0] = FLT_MAX;
	minVertex[1] = FLT_MAX;
	minVertex[2] = FLT_MAX;

	maxVertex [0] = FLT_MIN;
	maxVertex [1] = FLT_MIN;
	maxVertex [2] = FLT_MIN;

	for (int i = start; i < end; i++)
	{
		minVertex [0] = min(minVertex[0], vertz[indz[i].v[0]].pos[0]);
		minVertex [1] = min(minVertex [1], vertz[indz[i].v[0]].pos[1]);
		minVertex [2] = min(minVertex [2], vertz[indz[i].v[0]].pos[2]);

		maxVertex [0] = max(maxVertex [0], vertz[indz[i].v[0]].pos[0]);
		maxVertex [1] = max(maxVertex [1], vertz[indz[i].v[0]].pos[1]);
		maxVertex [2] = max(maxVertex [2], vertz[indz[i].v[0]].pos[2]);


		minVertex[0] = min(minVertex[0], vertz[indz[i].v[1]].pos[0]);
		minVertex[1] = min(minVertex[1], vertz[indz[i].v[1]].pos[1]);
		minVertex[2] = min(minVertex[2], vertz[indz[i].v[1]].pos[2]);

		maxVertex[0] = max(maxVertex[0], vertz[indz[i].v[1]].pos[0]);
		maxVertex[1] = max(maxVertex[1], vertz[indz[i].v[1]].pos[1]);
		maxVertex[2] = max(maxVertex[2], vertz[indz[i].v[1]].pos[2]);


		minVertex[0] = min(minVertex[0], vertz[indz[i].v[2]].pos[0]);
		minVertex[1] = min(minVertex[1], vertz[indz[i].v[2]].pos[1]);
		minVertex[2] = min(minVertex[2], vertz[indz[i].v[2]].pos[2]);

		maxVertex[0] = max(maxVertex[0], vertz[indz[i].v[2]].pos[0]);
		maxVertex[1] = max(maxVertex[1], vertz[indz[i].v[2]].pos[1]);
		maxVertex[2] = max(maxVertex[2], vertz[indz[i].v[2]].pos[2]);
	}

	aabb.center [0] = (maxVertex [0] + minVertex [0]) * 0.5f;
	aabb.center [1] = (maxVertex [1] + minVertex [1]) * 0.5f;
	aabb.center [2] = (maxVertex [2] + minVertex [2]) * 0.5f;

	aabb.extents [0] = (maxVertex [0] - minVertex [0]) * 0.5f;
	aabb.extents [1] = (maxVertex [1] - minVertex [1]) * 0.5f;
	aabb.extents [2] = (maxVertex [2] - minVertex [2]) * 0.5f;

	return aabb;

	return AABB();
}
