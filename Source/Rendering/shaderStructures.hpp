#pragma once
/* Shader Structures Copyright (C) by Mike Murrell 2017
These are all the structs that will go into the compute
shader, everything must be 16byte aligned
*/
#ifndef SHADERSTRUCTURES_HPP
#define SHADERSTRUCTURES_HPP
#endif // !PRIMITIVE

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
//#include "../pch.h"
#include "Components/renderComponents.hpp"

namespace Principia {
	struct ssGUI {
		glm::vec2 min;
		glm::vec2 extents;
		glm::vec2 alignMin;
		glm::vec2 alignExt;
		int layer;
		int id;
		int pad = 0;
		float alpha = 1.f;

		ssGUI() { min = glm::vec2(); extents = glm::vec2(); alignMin = glm::vec2(); alignExt = glm::vec2(); layer = 0; id = 0; pad = 0; alpha = 1.f; };
		ssGUI(glm::vec2 min, glm::vec2 max, glm::vec2 aMin, glm::vec2 aExt, int l, int i) : min(min), extents(max), alignMin(aMin), alignExt(aExt), layer(l), id(i) {};
	};
	//
	struct ssPrimitive {
		glm::mat4 world; //64bytes
		glm::vec3 extents; //12bytes
		int numChildren = 0; //4bytes;

		int id; //4bytes
		int matId; //4bytes
		int startIndex = 0;
		int endIndex = 0;

		ssPrimitive() { world = glm::mat4(); extents = glm::vec3(); id = 0; matId = 0; };
		ssPrimitive(PrimitiveComponent* pc) : world(pc->world), extents(pc->extents), numChildren(pc->numChildren),
			id(pc->id), matId(pc->matId), startIndex(pc->startIndex), endIndex(pc->endIndex)
		{};

	};//Total = 96bytes

	struct ssVert {
		glm::vec3 pos;
		float u;
		glm::vec3 norm;
		float v;
		ssVert() { pos = glm::vec3(); norm = glm::vec3(); u = 0; v = 0; };
		ssVert(const glm::vec3 &p, const glm::vec3 &n, const float &u, const float &v) : pos(p), norm(n), u(u), v(v) { };
	};

	struct ssTriangleIndex {
		glm::ivec3 v;	//12bytes
		int id;			//4bytes
		ssTriangleIndex() { v = glm::ivec3(); id = 0; };
		ssTriangleIndex(glm::ivec3 v, int id) : v(v), id(id) {};
		ssTriangleIndex(int v0, int v1, int v2, int id) : id(id) { v[0] = v0; v[1] = v1; v[2] = v2; };
	}; //Total = 16 bytes

	struct ssIndex {
		glm::ivec4 v; //16bytes
		ssIndex() { v = glm::ivec4(); };
		ssIndex(const glm::ivec4 &face) : v(face) {};
	};

	struct ssShape {
		glm::vec3 center;
		int matID = 0;
		glm::vec3 extents;
		int type;
		ssShape() { center = glm::vec3(); extents = glm::vec3(); type = 0; };
		ssShape(const glm::vec3& c, const glm::vec3 e, int t) : center(c), extents(e), type(t) {};
	}; //Total = 16bytes

	struct ssMaterial {
		glm::vec3 diffuse;
		float reflective;

		float roughness;
		float transparency;
		float refractiveIndex;
		int	  textureID;
		ssMaterial() { diffuse = glm::vec3(); reflective = 0; roughness = 0; transparency = 0; refractiveIndex = 0; textureID = 0; };
		//ssMaterial(glm::vec3 d, float m, float r) { diffuse = d, metallic = m; roughness = r; };
		ssMaterial(glm::vec3 d, float rfl, float ruf, float trn, float rfr, int ti) { diffuse = d; reflective = rfl; roughness = ruf; transparency = trn; refractiveIndex = rfr; textureID = ti; };
		//ssMaterial(glm::vec3 d, float m, float r, bool t, int id) { diffuse = d; metallic = m; roughness = r; hasTexture = b; textureID = id; };
	};	//32 bytes

	struct ssLight {
		glm::vec3 pos;
		float intensity;
		glm::vec3 color;
		int id;
		ssLight() { pos = glm::vec3(); intensity = 0; color = glm::vec3(); id = 0; };
	};

	/*
	static VkVertexInputBindingDescription getVertexBindingDescription() {
		//vertex info binding
		VkVertexInputBindingDescription vibd{};
		vibd.binding = 0;
		vibd.stride = sizeof(ssVert);
		vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vibd;
	}
	static std::array<VkVertexInputAttributeDescription, 4> getVertexAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> viad{};
		viad[0].binding = 0;
		viad[0].location = 0;
		viad[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		viad[0].offset = offsetof(ssVert, ssVert::pos);

		viad[1].binding = 0;
		viad[1].location = 1;
		viad[1].format = VK_FORMAT_R32_SFLOAT;
		viad[1].offset = offsetof(ssVert, ssVert::u);

		viad[2].binding = 0;
		viad[2].location = 2;
		viad[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		viad[2].offset = offsetof(ssVert, ssVert::norm);

		viad[3].binding = 0;
		viad[3].location = 3;
		viad[3].format = VK_FORMAT_R32_SFLOAT;
		viad[3].offset = offsetof(ssVert, ssVert::v);

		return viad;
	}*/

	static std::array<VkVertexInputBindingDescription, 2> getPrimitiveBindingDescriptions() 
	{
		std::array<VkVertexInputBindingDescription, 2> vibd{};

		//vertex info binding
		vibd[0].binding = 0;
		vibd[0].stride = sizeof(ssVert);
		vibd[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		//vertex info binding
		vibd[1].binding = 1;
		vibd[1].stride = sizeof(ssPrimitive);
		vibd[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		return vibd;
	}

	static std::array<VkVertexInputAttributeDescription, 11> getPrimitiveAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 11> viad{};

		viad[0].binding = 0;
		viad[0].location = 0;
		viad[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		viad[0].offset = offsetof(ssVert, ssVert::pos);

		viad[1].binding = 0;
		viad[1].location = 1;
		viad[1].format = VK_FORMAT_R32_SFLOAT;
		viad[1].offset = offsetof(ssVert, ssVert::u);

		viad[2].binding = 0;
		viad[2].location = 2;
		viad[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		viad[2].offset = offsetof(ssVert, ssVert::norm);

		viad[3].binding = 0;
		viad[3].location = 3;
		viad[3].format = VK_FORMAT_R32_SFLOAT;
		viad[3].offset = offsetof(ssVert, ssVert::v);

		viad[4].binding = 1;
		viad[4].location = 4;
		viad[4].format = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT;
		viad[4].offset = offsetof(ssPrimitive, ssPrimitive::world);

		viad[5].binding = 1;
		viad[5].location = 5;
		viad[5].format = VK_FORMAT_R32G32B32_SFLOAT;
		viad[5].offset = offsetof(ssPrimitive, ssPrimitive::extents);
		
		viad[6].binding = 1;
		viad[6].location = 6;
		viad[6].format = VK_FORMAT_R32_SINT;
		viad[6].offset = offsetof(ssPrimitive, ssPrimitive::numChildren);

		viad[7].binding = 1;
		viad[7].location = 7;
		viad[7].format = VK_FORMAT_R32_SINT;
		viad[7].offset = offsetof(ssPrimitive, ssPrimitive::id);

		viad[8].binding = 1;
		viad[8].location = 8;
		viad[8].format = VK_FORMAT_R32_SINT;
		viad[8].offset = offsetof(ssPrimitive, ssPrimitive::matId);

		viad[9].binding = 1;
		viad[9].location = 9;
		viad[9].format = VK_FORMAT_R32_SINT;
		viad[9].offset = offsetof(ssPrimitive, ssPrimitive::startIndex);

		viad[10].binding = 1;
		viad[10].location = 10;
		viad[10].format = VK_FORMAT_R32_SINT;
		viad[10].offset = offsetof(ssPrimitive, ssPrimitive::endIndex);

		return viad;
	}
}