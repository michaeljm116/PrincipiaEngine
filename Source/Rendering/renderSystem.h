#pragma once
/* Render System Copyright (C) by Mike Murrell 2017
everything here is so convoluted and rushed it needs
much rework but brotha aint got time fo dat
*/


#include "../pch.h"
#include "rendermanagers.h"
#include "renderbase.h"
#include "shaderStructures.hpp"
#include "engineUISystem.h"
#include "../Game/camera.hpp"
#include "../Game/script.hpp"
#include "../Utility/window.h"
#include "../Utility/bvhComponent.hpp"

static const int MAXTEXTURES = 5;

class RenderSystem : public RenderBase, public artemis::EntityProcessingSystem
{
public:
	RenderSystem();
	~RenderSystem();

	void preInit();
	void initialize();
	void mainLoop();
	void drawFrame();
	void updateUniformBuffer();
	void processEntity(artemis::Entity &e);
	void end();

	void loadResources();

	void addLight(artemis::Entity &e);
	void addCamera(artemis::Entity &e);
	void addMaterial(glm::vec3 diff, float rfl, float rough, float trans, float ri);

	void addNodes(std::vector<NodeComponent*> nodes);
	void addNode(NodeComponent* node);
	void updateGui(GUIComponent* gc);
	void addGuiNumber(GUINumberComponent* gnc);
	void updateGuiNumber(GUINumberComponent* gnc);

	void deleteMesh(NodeComponent* node);
	void deleteNode(NodeComponent* node);

	void updateObjectMemory();
	void updateJointMemory();
	void updateMeshMemory();
	void updateGeometryMemory(ObjectType type);

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
	int flattenBVH(std::shared_ptr<BVHNode> node, int* offset);
	//void updateLight(LightComponent* l);

	virtual void cleanup();
	virtual void cleanupSwapChain();
	virtual void recreateSwapChain();

	//std::vector<ssVert>& getVertices() { return verts; };
	//std::vector<ssMesh>& getMeshes() { return meshes; };
	//ssMesh& getMesh(int i) { return meshes[i]; };
	ssJoint& getJoint(int i) { return joints[i]; };
	ssPrimitive& getObject(int i) { return objects[i]; };
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
			VBuffer<ssVert> verts;			// (Shader) storage buffer object with scene verts
			VBuffer<ssIndex> faces;			// (Shader) storage buffer object with scene indices
			VBuffer<ssShape> shapes;		// for animatied shapes 
			VBuffer<ssPrimitive> objects;	// for the primitives
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
			glm::vec3 pos = glm::vec3(0.0f, 0.0f, 4.0f);
			float aspectRatio;
			glm::vec3 lookat = glm::vec3(0.0f, 0.5f, 0.0f);
			float fov = 10.0f;
		} ubo;
		VBuffer<UBOCompute> uniformBuffer;			// Uniform buffer object containing scene data
	} compute;

	std::vector<ssPrimitive> objects;
	std::vector<ssJoint> joints;
	std::vector<ssMaterial> materials;
	std::vector<ssLight> lights;
	std::vector<ssGUI> guis;
	std::vector<ssBVHNode> bvh;


	std::vector<MeshComponent*> meshComps;
	std::vector<PrimitiveComponent*> objectComps;
	std::vector<JointComponent*> jointComps;
	std::vector<LightComponent*> lightComps;

	std::unordered_map<int32_t, std::pair<int,int>> meshAssigner;
	std::unordered_map<int32_t, std::pair<int, int>> jointAssigner;
	std::unordered_map<int32_t, std::pair<int, int>> shapeAssigner;

	Scripto testScript;


	void prepareStorageBuffers();
	void createUniformBuffers();
	void prepareTextureTarget(Texture *tex, uint32_t width, uint32_t height, VkFormat format);
	bool prepared = false;

	Camera m_Cam;
	Timer m_RenderTime;

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
	EngineUISystem* ui;
	void setupUI();
	VkExtent2D scaledSwap;
	void swapRatio(float f);
	public:
	void updateMaterials();
	void updateMaterial(int id);
	void togglePlayMode(bool pm);

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
