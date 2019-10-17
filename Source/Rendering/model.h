#pragma once
/*Copyright (C) by Mike Murrell 
this is legacy atm, might be useful if
i ever do a hybrid approach*/

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif // !GLM_FORCE_RADIANS

//#include "../pch.h"
#include "VulkanInitializers.hpp"
#include <glm/glm.hpp>
#include <array>
//#include"../Game/transform.hpp"

//Regular Vertex
struct Vertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec3 tang;
	glm::vec2 uv;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
		//The Vertex Position
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = 0;// offsetof(Vertex, pos);
		//The Normal Position
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = sizeof(float) * 3;// offsetof(Vertex, norm);
		//The Tangent Position
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = sizeof(float) * 6;// offsetof(Vertex, tang);
		//The Texture Coordinate
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = sizeof(float) * 9;// offsetof(Vertex, uv);

		return attributeDescriptions;
	}
	bool operator==(const Vertex& other) const {
		return pos == other.pos && norm == other.norm && tang == other.tang && uv == other.uv;
	}
};
