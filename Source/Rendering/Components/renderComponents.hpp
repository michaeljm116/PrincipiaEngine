#pragma once

#include <Artemis/Artemis.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>

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
		RenderType type = RENDER_NONE;
		RendererType renderer = kComputeRaytracer;
		RenderComponent() = default;
		RenderComponent(RenderType t) : type(t) {}
	};

	struct SphereComponent : artemis::Component {
		float radius = 0.0f;
		int sphereIndex = 0;
		SphereComponent() = default;
		SphereComponent(float r) : radius(r) {}
	};

	struct BoxComponent : artemis::Component {
		glm::vec3 center{};
		glm::vec3 extents{};
		int boxIndex = 0;
		BoxComponent() = default;
		BoxComponent(glm::vec3 c, glm::vec3 e) : center(c), extents(e) {}
	};

	struct CylinderComponent : artemis::Component {
		glm::vec3 top{};
		glm::vec3 bottom{};
		float radius = 0.0f;
		int cylinderIndex = 0;
		CylinderComponent() = default;
		CylinderComponent(const glm::vec3& t, const glm::vec3& b, float r) : top(t), bottom(b), radius(r) {}
	};

	struct PlaneComponent : artemis::Component {
		glm::vec3 normal{};
		float distance = 0.0f;
		int planeIndex = 0;
		PlaneComponent() = default;
		PlaneComponent(const glm::vec3& n, float d) : normal(n), distance(d) {}
	};

	struct MeshComponent : artemis::Component {
		int meshIndex = 0;
		int meshModelID = 0;
		int meshResourceIndex = 0;
		int uniqueID = 0;
		MeshComponent() = default;
		explicit MeshComponent(int si) : meshIndex(si) {}
		MeshComponent(int id, int ri) : meshModelID(id), meshResourceIndex(ri) {}
	};

	struct PrimitiveComponent : artemis::Component {
		glm::mat4 world{};
		glm::vec3 extents{};
		glm::vec3 aabbExtents{};
		int numChildren = 0;
		int id = 0;
		int matId = 0;
		int startIndex = 0;
		int endIndex = 0;
		PrimitiveComponent() = default;
		explicit PrimitiveComponent(int i) : id(i) {}
		inline glm::vec3 center() const { return glm::vec3(world[3].x, world[3].y, world[3].z); }
	};

	struct ModelComponent : artemis::Component {
		int modelIndex = 0;
		int modelUniqueID = 0;
		ModelComponent() = default;
		explicit ModelComponent(int n) : modelUniqueID(n) {}
		ModelComponent(int n, int id) : modelIndex(n), modelUniqueID(id) {}
	};

	enum class SelectableState {
		Unselected,
		Released,
		Held,
		Pressed
	};

	struct Cmp_Selectable : artemis::Component {
		SelectableState state = SelectableState::Unselected;
		bool active = false;
		bool reset = false;
	};

	struct GUIComponent : artemis::Component {
		glm::vec2 min{};
		glm::vec2 extents{};
		glm::vec2 alignMin{};
		glm::vec2 alignExt{};
		int layer = 0;
		int id = 0;
		int ref = 0;
		float alpha = 0.f;
		bool update = true;
		GUIComponent() = default;
		GUIComponent(const glm::vec2& m, const glm::vec2& e, const glm::vec2& amin, const glm::vec2& ae, int l, int i, float a) :
			min(m), extents(e), alignMin(amin), alignExt(ae), layer(l), id(i), alpha(a) {}
	};

	struct GUINumberComponent : GUIComponent {
		int number = 0;
		int highest_active_digit_index = 0;
		std::vector<int> shaderReferences;
		GUINumberComponent() = default;
		GUINumberComponent(const glm::vec2& m, const glm::vec2& e, int n) : GUIComponent(m, e, glm::vec2(0.0f), glm::vec2(0.1f, 1.f), 0, 0, 0.f), number(n) {}
		GUINumberComponent(const glm::vec2& m, const glm::vec2& e, int n, float a) : GUIComponent(m, e, glm::vec2(0.0f), glm::vec2(0.1f, 1.f), 0, 0, a), number(n) {}
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