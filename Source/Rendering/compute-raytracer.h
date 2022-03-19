#pragma once
#include "renderer.h"

#include "../Utility/bvhComponent.hpp"
#include <unordered_map>

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

#pragma Compute exclusive Functions
		void UpdateBVH(std::vector<artemis::Entity*>& ordredPrims, std::shared_ptr<BVHNode> root, int numNodes);
		int FlattenBVH(std::shared_ptr<BVHNode> node, int* offset, std::vector<ssBVHNode>& bvh);
#pragma endregion

		bool editor_ = true;
		ssLight& getLight(int i) { return lights_[i]; }

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
			for (int i = temp.size() - 1; i > -1; --i)
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
			VkDescriptorSetLayout descriptor_set_layout;
			VkDescriptorSet descriptor_set;
			VkPipelineLayout pipeline_layout;
			VkPipeline pipeline;
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

			VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
			VkCommandPool command_pool;					// Use a separate command pool (queue family may differ from the one used for graphics)
			VkCommandBuffer command_buffer;				// Command buffer storing the dispatch commands and barriers
			VkFence fence;								// Synchronization fence to avoid rewriting compute CB if still in use
			VkDescriptorSetLayout descriptor_set_layout;	// Compute shader binding layout
			VkDescriptorSet descriptor_set;				// Compute shader bindings
			VkPipelineLayout pipeline_layout;			// Layout of the compute pipeline
			VkPipeline pipeline;						// Compute raytracing pipeline
			struct UBOCompute {							// Compute shader uniform block object
				glm::mat4 rotM = glm::mat4(1);
				float fov = 10.0f;
				float aspect_ratio;
				int rand;
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

		void CreateComputeCommandBuffer();
		void PrepareCompute();
		void DestroyCompute();

		VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags flags);
		std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets_;
	};
}