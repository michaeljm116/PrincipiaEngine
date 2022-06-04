#pragma once
#ifndef RENDERCOMPONENTS_HPP
#define RENDERCOMPONENTS_HPP

#include <Artemis/Artemis.h>
#include <glm/glm.hpp>
#include <vector>
#include "vulkan/vulkan.h"
#include <array>

//enum class ObjectType {
//	Sphere,
//	Box,
//	Plane,
//	Cylinder,
//	Cone,
//	Mesh,
//	Light,
//	Camera,
//	None
//};

namespace Principia {

	enum RenderType {
		RENDER_NONE = 0x00,
		RENDER_MATERIAL = 0x01,
		RENDER_PRIMITIVE = 0x02,
		RENDER_LIGHT = 0x04,
		RENDER_GUI = 0x08,
		RENDER_GUINUM = 0x10,
		RENDER_CAMERA = 0x20
	};

	enum RendererType {
		kComputeRaytracer,
		kHardwareRaytracer,
		kComputeRasterizer,
		kHardwareRasterizer
	};

	struct RenderComponent : artemis::Component {
		RenderType type;
		RendererType renderer = kComputeRaytracer;
		RenderComponent() {};
		RenderComponent(RenderType t) : type(t) {};
	};

	struct SphereComponent : artemis::Component {
		float radius;
		int sphereIndex;

		SphereComponent() {};
		SphereComponent(float r) { radius = r; };
	};

	struct BoxComponent : artemis::Component {
		glm::vec3 center;
		glm::vec3 extents;
		int boxIndex;

		BoxComponent() {};
		BoxComponent(glm::vec3 c, glm::vec3 e) : center(c), extents(e) {};
	};

	struct CylinderComponent : artemis::Component {
		glm::vec3 top;
		glm::vec3 bottom;
		float radius;
		int cylinderIndex;

		CylinderComponent() {};
		CylinderComponent(glm::vec3 t, glm::vec3 b, float r) : top(t), bottom(b), radius(r) {};
	};

	struct  PlaneComponent : artemis::Component {
		glm::vec3 normal;
		float distance;
		int planeIndex;

		PlaneComponent() {};
		PlaneComponent(glm::vec3 n, float d) :normal(n), distance(d) {};
	};

	struct MeshComponent : artemis::Component {
		int meshIndex;
		int meshModelID;
		int meshResourceIndex;
		int uniqueID;

		MeshComponent() {};
		MeshComponent(int si) { meshIndex = si; };
		MeshComponent(int id, int ri) : meshModelID(id), meshResourceIndex(ri) {};
	};

	struct PrimitiveComponent : artemis::Component {
		glm::mat4 world; //64bytes
		glm::vec3 extents; //12bytes
		glm::vec3 aabbExtents; //12bytes
		int numChildren = 0; //4bytes;
		
		int id; //4bytes
		int matId; //4bytes
		int startIndex = 0;
		int endIndex = 0;


		PrimitiveComponent() {};
		PrimitiveComponent(int i) : id(i) {};
		inline glm::vec3 center() { return glm::vec3(world[3].x, world[3].y, world[3].z); };
	};

	struct ModelComponent : artemis::Component {
		int modelIndex;
		int modelUniqueID;

		ModelComponent() {};
		ModelComponent(int n) { modelUniqueID = n; };
		ModelComponent(int n, int id) { modelIndex = n; modelUniqueID = id; };
	};

	enum class SelectableState {
		Unselected, 
		Released, 
		Held, 
		Pressed
	};
	struct Cmp_Selectable : artemis::Component {
		SelectableState state;
		bool active = false;
		bool reset = false;
	};

	struct GUIComponent : artemis::Component {
		glm::vec2 min;
		glm::vec2 extents;
		glm::vec2 alignMin;
		glm::vec2 alignExt;
		int layer;
		int id;
		int ref;
		float alpha = 0.f;
		//bool visible;
		GUIComponent() {};
		GUIComponent(glm::vec2 m, glm::vec2 e, glm::vec2 amin, glm::vec2 ae, int l, int i, float a) :
			min(m), extents(e), alignMin(amin), alignExt(ae), layer(l), id(i), alpha(a) {};
	};
	struct GUINumberComponent : GUIComponent {
		int number;
		std::vector<int> shaderReferences;
		GUINumberComponent() {};
		GUINumberComponent(glm::vec2 m, glm::vec2 e, int n) { min = m; extents = e; number = n; alignMin = glm::vec2(0.0f, 0.0f); alignExt = glm::vec2(0.1f, 1.f); layer = 0; id = 0; };
		GUINumberComponent(glm::vec2 m, glm::vec2 e, int n, float a) { min = m; extents = e; number = n; alignMin = glm::vec2(0.0f, 0.0f); alignExt = glm::vec2(0.1f, 1.f); layer = 0; id = 0; alpha = a; };

	};

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

}

#endif // !GEOMETRY