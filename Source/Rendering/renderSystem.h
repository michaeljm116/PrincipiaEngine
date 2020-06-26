#pragma once
/* Render System Copyright (C) by Mike Murrell 2017
everything here is so convoluted and rushed it needs
much rework but brotha aint got time fo dat
*/

#include "rendermanagers.h"
#include "renderbase.h"
#include "shaderStructures.hpp"
#include "../Game/script.hpp"
#include "../Utility/window.h"
#include "../Utility/bvhComponent.hpp"
#include "../Utility/timer.h"

#include "../Utility/componentIncludes.h"
#include <unordered_map>

namespace Principia {
	static const int MAXTEXTURES = 5;

	class RenderSystem : public RenderBase, public artemis::EntityProcessingSystem
	{
	public:
		RenderSystem();
		~RenderSystem();
		void preInit();
		void initialize();
		std::vector<VkFramebuffer>& getFrameBuffers() {
			return swapChainFramebuffers;
		};
		VkExtent2D& getSwapChainExtent() {
			return swapChainExtent;
		}
		VkDeviceInfo devInfo = {};
		VkDeviceInfo updateDeviceInfo();
		VkSubmitInfo submitInfo = {};
		void startFrame(uint32_t& imageIndex);
		void endFrame(const uint32_t& imageIndex);
		void updateUniformBuffer();
		void processEntity(artemis::Entity &e);

		void added(artemis::Entity &e) override;
		void removed(artemis::Entity &e) override;
		void end() override;

		void loadResources();

		void addLight(artemis::Entity &e);
		void addCamera(artemis::Entity &e);
		void addMaterial(glm::vec3 diff, float rfl, float rough, float trans, float ri);

		void addNodes(std::vector<NodeComponent*> nodes);
		void addNode(NodeComponent* node);
		void updateGui(GUIComponent* gc);
		void addGuiNumber(GUINumberComponent* gnc);
		void updateGuiNumber(GUINumberComponent* gnc);


		void updateObjectMemory();
		void updateJointMemory();

		float getRenderTime() { return m_RenderTime.ms; }

		enum RenderUpdate {
			UPDATE_BOX = 0x01,
			UPDATE_SPHERE = 0x02,
			UPDATE_PLANE = 0x04,
			UPDATE_CYLINDER = 0x08,
			UPDATE_CONE = 0x10,
			UPDATE_MESH = 0x20,
			UPDATE_MATERIAL = 0x40,
			UPDATE_NONE = 0x80,
			UPDATE_OBJECT = 0x100,
			UPDATE_LIGHT = 0x200,
			UPDATE_GUI = 0x400,
			UPDATE_JOINT = 0x800
		};

		int32_t updateflags;
		void setRenderUpdate(RenderUpdate ru) {
			updateflags |= ru;
			if (updateflags & UPDATE_NONE)
				updateflags &= ~UPDATE_NONE;
		};
		void updateBuffers();
		void updateCamera(CameraComponent* c);
		void updateBVH(std::vector<artemis::Entity*>& ordredPrims, std::shared_ptr<BVHNode> root, int numNodes);
		int flattenBVH(std::shared_ptr<BVHNode> node, int* offset, std::vector<ssBVHNode>& bvh);

		virtual void cleanup();
		virtual void cleanupSwapChain();
		virtual void recreateSwapChain();
		bool editor = true;
		void togglePlayMode(bool b);
		ssJoint& getJoint(int i) { return joints[i]; };
		ssLight& getLight(int i) { return lights[i]; };

	private:

		artemis::ComponentMapper<RenderComponent> renderMapper;

		void SetStuffUp();
		void createGraphicsPipeline();
		void createDescriptorPool();
		void createDescriptorSets();
		void createDescriptorSetLayout();
		void createCommandBuffers(float swapratio, int32_t offsetWidth, int32_t offsetHeight);
		void updateDescriptors();

		VkDescriptorPool	  descriptorPool;
		struct {
			VkDescriptorSetLayout	descriptorSetLayout;
			VkDescriptorSet			descriptorSetPreCompute;
			VkDescriptorSet			descriptorSet;
			VkPipelineLayout		pipelineLayout;
			VkPipeline				pipeline;
		}graphics;

		// Resources for the compute part of the example
		struct {
			struct {
				//GPU READ ONLY
				VBuffer<ssVert> verts;			// (Shader) storage buffer object with scene verts
				VBuffer<ssIndex> faces;			// (Shader) storage buffer object with scene indices
				VBuffer<ssBVHNode> blas;		// (Shader) storage buffer object with bottom level acceleration structure
				VBuffer<ssShape> shapes;		// for animatied shapes 

				//CPU + GPU 
				VBuffer<ssPrimitive> primitives;	// for the primitives
				VBuffer<ssJoint> joints;		// for the animated joints
				VBuffer<ssMaterial> materials;	// (Shader) storage buffer object with scene Materials
				VBuffer<ssLight> lights;
				VBuffer<ssGUI> guis;
				VBuffer<ssBVHNode> bvh;			// for the bvh bruh

			} storageBuffers;

			VkQueue queue;								// Separate queue for compute commands (queue family may differ from the one used for graphics)
			VkCommandPool commandPool;					// Use a separate command pool (queue family may differ from the one used for graphics)
			VkCommandBuffer commandBuffer;				// Command buffer storing the dispatch commands and barriers
			VkFence fence;								// Synchronization fence to avoid rewriting compute CB if still in use
			VkDescriptorSetLayout descriptorSetLayout;	// Compute shader binding layout
			VkDescriptorSet descriptorSet;				// Compute shader bindings
			VkPipelineLayout pipelineLayout;			// Layout of the compute pipeline
			VkPipeline pipeline;						// Compute raytracing pipeline
			struct UBOCompute {							// Compute shader uniform block object
				glm::mat4 rotM = glm::mat4(1);
				float fov = 10.0f;
				float aspectRatio;
			} ubo;
			VBuffer<UBOCompute> uniformBuffer;			// Uniform buffer object containing scene data
		} compute;

		std::vector<ssPrimitive> primitives;
		std::vector<ssJoint> joints;
		std::vector<ssMaterial> materials;
		std::vector<ssLight> lights;
		std::vector<ssGUI> guis;
		std::vector<ssBVHNode> bvh;


		std::vector<MeshComponent*> meshComps;
		//std::vector<PrimitiveComponent*> objectComps;
		//std::vector<JointComponent*> jointComps;
		std::vector<LightComponent*> lightComps;

		std::unordered_map<int32_t, std::pair<int, int>> meshAssigner;
		std::unordered_map<int32_t, std::pair<int, int>> jointAssigner;
		std::unordered_map<int32_t, std::pair<int, int>> shapeAssigner;
		

		void prepareStorageBuffers();
		void createUniformBuffers();
		void prepareTextureTarget(Texture *tex, uint32_t width, uint32_t height, VkFormat format);
		bool prepared = false;

		Camera m_Cam;
		Principia::Timer m_RenderTime = Timer("Render");

		Texture			computeTexture;
		Texture			guiTextures[MAXTEXTURES];

		void			createComputeCommandBuffer();
		void			prepareCompute();
		void			destroyCompute();

		//uint32_t VkDescriptorType uint32_t VkShaderStageFlags;
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags flags);
		std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets;
		//UI
		//EngineUI *ui = nullptr;
		VkExtent2D scaledSwap;
		void swapRatio(float f);
	public:
		void updateMaterials();
		void updateMaterial(int id);

		std::vector<int> intToArrayOfInts(const int &a) {
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

	};

}