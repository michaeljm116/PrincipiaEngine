#pragma once
/* Engine UI System Copyright (C) by Mike Murrell 

*/

#include "../../Lib/imgui/imgui.h"
#include "rendermanagers.h"
#include "VulkanBuffer.hpp"
#include "../ArtemisFrameWork/Artemis/Artemis.h"
#include "../Utility/componentIncludes.h"
#include "../Utility/resourceManager.h"
#include "../Utility/componentIncludes.h"

struct UIOverlayCreateInfo
{
	VulkanDevice *device;
	VkQueue copyQueue;
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;
	VkFormat colorformat;
	VkFormat depthformat;
	uint32_t width;
	uint32_t height;
	std::vector<VkPipelineShaderStageCreateInfo> shaders;
	VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	uint32_t subpassCount = 1;
	std::vector<VkClearValue> clearValues = {};
	uint32_t attachmentCount = 1;
};

enum class EditState {
	Translate, Rotate, Scale, None
};

enum class CreateState {
	Model, Sphere, Cylinder, Box, Light, Material, Scene
};

class EngineUISystem : public artemis::EntityProcessingSystem
{
#pragma region setup
private:
	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	//VBuffer<ImDrawVert> vertexBuffer;
	//VBuffer<ImDrawIdx> indexBuffer;
	int32_t vertexCount = 0;
	int32_t indexCount = 0;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline pipeline;
	VkRenderPass renderPass;
	VkCommandPool commandPool;
	VkFence fence;

	VkDeviceMemory fontMemory = VK_NULL_HANDLE;
	VkImage fontImage = VK_NULL_HANDLE;
	VkImageView fontView = VK_NULL_HANDLE;
	VkSampler sampler;

	struct PushConstBlock {
		glm::vec2 scale;
		glm::vec2 translate;
	} pushConstBlock;

	UIOverlayCreateInfo createInfo = {};

	void prepareResources();
	void preparePipeline();
	void prepareRenderPass();
	void updateCommandBuffers();

	artemis::ComponentMapper<NodeComponent> nodeMapper;

public:
	bool visible = false;
	float scale = 1.0f;

	std::vector<VkCommandBuffer> cmdBuffers;

	EngineUISystem();
	~EngineUISystem();
	void init(UIOverlayCreateInfo createInfo);
	void processEntity(artemis::Entity& e) {};
	
	void CleanUp();
	void update();
	void resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers);

	void submit(VkQueue queue, uint32_t bufferindex, VkSubmitInfo submitInfo);
	//void addParentEntity(artemis::Entity * e, std::string name);

#pragma endregion

	////////// CALLBACKS ///////////////////
	bool header(const char* caption);
	bool checkBox(const char* caption, bool* value);
	bool checkBox(const char* caption, int32_t* value);
	bool inputFloat(const char* caption, float* value, float step, uint32_t precision);
	bool sliderFloat(const char* caption, float* value, float min, float max);
	bool sliderInt(const char* caption, int32_t* value, int32_t min, int32_t max);
	bool comboBox(const char* caption, int32_t* itemindex, std::vector<std::string> items);
	bool listBox(const char* caption, int32_t* itemindex, std::vector<std::string> items);
	bool modelBox(const char* caption, int32_t* itemindex, std::vector<rModel> items);
	bool matBox(const char* caption, int32_t* itemindex, std::vector<rMaterial> items);
	bool button(const char* caption);
	void text(const char* formatstr, ...);


	///////////// THE ACTUAL UI STUFF 

	
private:



	int32_t itemIndex;
	int32_t modelIndex;
	int32_t matIndex;
	int32_t animIndex = 0;
	int32_t numAnims = 0;
	//std::vector<std::string> matNames;
	int counter = 0;
	bool cleaned = false;


	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 sca = glm::vec3(1.f);

	//std::vector<NodeComponent*> parents;
	//std::vector<std::string> parentNames;
	int32_t parentIndex = 0;
	int32_t childrenIndex = 0;
	std::vector<int32_t> parentIndexes;
	//std::vector<int> selection_mask;// = (1 << 2);
	
	NodeComponent* activeNode;
	TransformComponent* activeTransform;
	MaterialComponent*	activeMaterial;
	LightComponent* activeLight;
	NodeComponent* activeCamera;
	AnimationComponent* activeAnimation;
	CharacterComponent* activeController;
	//ObjectType activeType;

	bool hasTransform = false;
	bool hasMaterial = false;
	bool hasLight = false;
	bool hasCamera = false;
	bool hasChildren = false;
	bool hasSphere = false;
	bool hasBox = false;
	bool isDynamic = true;

	int numComponents = 0;

	bool menutest = true;
	EditState eState = EditState::Translate;
	int32_t menuHeight = 36;
	int32_t bottomHeight;
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

	//RenderSystem* rs;
public:
	void topSection(float w, float h, bool* p_open);	//Menu 
	void bottomSection(float w, float h);				//Editor?
	void leftSection(float w, float h);					//Resources
	void rightSection(float w, float h);				//Scene Graph

	void updateOverlay();
	void updateMaterials();
	void updateActiveNode(NodeComponent* n);
	void updateInput();

	bool renderNodes(std::vector<NodeComponent*>& nodes, int lvl);
	void componentVerify();

	void editNode();
	void editTransform();
	void editGeometry();
	void editMaterial();
	void editLight();
	void editCamera();
	void editController();

	void createModel(bool& p_create);
	void createSphere(bool& p_create);
	void createBox(bool& p_create);
	void createPlane(bool& p_create);
	void createCylinder(bool& p_create);
	void createMaterial(bool& p_create);

	//void translate(glm::vec3 v);
	void materialSelect();
	void animationSelect();
	void setActiveNode(NodeComponent* n) { activeNode = n; };
	void findActiveCamera();
	void setActiveAsCamera();
	void moveCameraToNode();

	void refresh();
};

/*
have a list of parents
for each parent, if has child
display children

okay so like how to do activenode thingy tho...?
if selected....
find parent index
if children find c hilden index
then you can like set active node by just... parent index or if child then parent + child index
*/