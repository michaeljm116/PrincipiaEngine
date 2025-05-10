#pragma once
#include "renderer.h"

#include "../Utility/bvhComponent.hpp"
#include <unordered_map>


struct RTCBuildPrimitive;

namespace Principia {

	class ComputeRaytracer : public Renderer
	{
	public:
		ComputeRaytracer();
		~ComputeRaytracer();

		void StartUp(artemis::World* world) override;
		void Initialize(artemis::ComponentMapper<RenderComponent>* render_mapper) override;
		void StartFrame(uint32_t& image_index) override;
		void EndFrame(const uint32_t& image_index) override;

		void Added(artemis::Entity& e) override;
		void Removed(artemis::Entity& e) override;
		void ProcessEntity(artemis::Entity& e) override;
		void End() override;

		void CleanUp() override;
		void CleanUpSwapChain() override;
		void RecreateSwapChain() override;


		void UpdateUniformBuffer();
		void LoadResources();

#pragma region System Related functions
		void AddMaterial(glm::vec3 diff, float rfl, float rough, float trans, float ri) override;
		void AddNode(NodeComponent* node) override;
		void UpdateMaterial(int id) override;

		//meh
		void UpdateGui(GUIComponent* gc);
		void AddGuiNumber(GUINumberComponent* gnc);
		void UpdateGuiNumber(GUINumberComponent* gnc);
		
		void UpdateBuffers();
		void UpdateCamera(CameraComponent* c) override;

		void TogglePlayMode(bool b) override;
#pragma endregion 

#pragma region Compute exclusive Functions
		void UpdateBVH(const std::vector<RTCBuildPrimitive>& ordered_prims, const std::vector<artemis::Entity*>& prims, BvhNode* root, int num_nodes);
		int FlattenBVH(BvhNode* node, const BvhBounds& bounds, int* offset, std::vector<ssBVHNode>& bvh);
		std::vector<int> ordered_prims_map;
#pragma endregion

		bool editor_ = true;
		ssLight& getLight(int i) {
			for (auto& l : lights_) {
				if (i == l.id)
					return l;
			}
			return lights_[0];
		}

		std::vector<int> intToArrayOfInts(const int& a) {
			if (a == 0) {
				std::vector<int> zro;
				zro.push_back(0);
				return zro;
			}
			std::vector<int> temp;
			int c = a;
			while (c > 0) {
				temp.push_back(c % 10);
				c /= 10;
			}
			std::vector<int> res;
			for (int i = static_cast<int>(temp.size()) - 1; i > -1; --i)
				res.push_back(temp[i]);
			return res;
		}

	private:

		void SetStuffUp();
		void CreateGraphicsPipeline();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateDescriptorSetLayout();
		void CreateCommandBuffers(float swap_ratio, int32_t offset_width, int32_t offset_height);
		void UpdateDescriptors() override;

		VkDescriptorPool descriptor_pool_;
		struct {
			VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
			VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
			VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
			VkPipeline pipeline = VK_NULL_HANDLE;
		}graphics_;

		struct
		{
			struct {
				//GPU READ ONLY
				VBuffer<ssVert> verts;			// (Shader) storage buffer object with scene verts
				VBuffer<ssIndex> faces;			// (Shader) storage buffer object with scene indices
				VBuffer<ssBVHNode> blas;		// (Shader) storage buffer object with bottom level acceleration structure
				VBuffer<ssShape> shapes;		// for animatied shapes 

				//CPU + GPU 
				VBuffer<ssPrimitive> primitives;	// for the primitives
				VBuffer<ssMaterial> materials;	// (Shader) storage buffer object with scene Materials
				VBuffer<ssLight> lights;
				VBuffer<ssGUI> guis;
				VBuffer<ssBVHNode> bvh;			// for the bvh bruh

			} storage_buffers;

			VkQueue queue = VK_NULL_HANDLE;
			VkCommandPool command_pool = VK_NULL_HANDLE;
			VkCommandBuffer command_buffer = VK_NULL_HANDLE;
			VkFence fence = VK_NULL_HANDLE;
			VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
			VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
			VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
			VkPipeline pipeline = VK_NULL_HANDLE;
			struct UBOCompute {
				glm::mat4 rotM = glm::mat4(1);
				float fov = 10.0f;
				float aspect_ratio = 1.0f; // Assuming 1.0 as default aspect ratio
				int rand = 0;  // Assuming 0 as default random value
			} ubo;
			VBuffer<UBOCompute> uniform_buffer;			// Uniform buffer object containing scene data
		} compute_;

		std::vector<ssPrimitive> primitives_;
		std::vector<ssMaterial> materials_;
		std::vector<ssLight> lights_;
		std::vector<ssGUI> guis_;
		std::vector<ssBVHNode> bvh_;


		std::vector<MeshComponent*> mesh_comps_;
		//std::vector<PrimitiveComponent*> objectComps;
		//std::vector<JointComponent*> jointComps;
		std::vector<LightComponent*> light_comps_;

		std::unordered_map<int32_t, std::pair<int, int>> mesh_assigner_;
		std::unordered_map<int32_t, std::pair<int, int>> joint_assigner_;
		std::unordered_map<int32_t, std::pair<int, int>> shape_assigner_;

		void PrepareStorageBuffers();
		void CreateUniformBuffers();
		void PrepareTextureTarget(Texture* tex, uint32_t width, uint32_t height, VkFormat format);
		bool prepared_ = false;

		Texture compute_texture_;
		Texture gui_textures_[MAX_TEXTURES];
		std::vector<Texture> bindless_textures;

		void CreateComputeCommandBuffer();
		void PrepareCompute();
		void DestroyCompute();

		VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags flags);
		std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets_;

	};
}