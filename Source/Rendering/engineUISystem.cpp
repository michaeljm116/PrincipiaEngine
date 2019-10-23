/*
*
* EngineUISystem Copyright (C) 2017 by Mike Murrell
* highly modified from sascha williems ui overlay
*/
#include "EngineUISystem.h"
#include "../Game/Application/applicationComponents.h"
#include "VulkanInitializers.hpp"
#include "../Utility/resourceManager.h"
#include "renderSystem.h"
#include <omp.h>
#include "../Game/scene.h"
#include "../Utility/Input.h"

#pragma region setupstuff
EngineUISystem::EngineUISystem()
{
	addComponentType<EditorComponent>();
	addComponentType<GlobalController>();
}

/** Free up all Vulkan resources acquired by the UI overlay */
EngineUISystem::~EngineUISystem()
{
	if(!cleaned)
	CleanUp();
}
void EngineUISystem::init(UIOverlayCreateInfo createInfo)
{
	//idk
	//itemIndex = 0;
	//for (int i = 0; i < RESOURCEMANAGER.getMaterials().size(); ++i) {
	//	rMaterial mat = RESOURCEMANAGER.getMaterial(i);
	//	//materials.push_back(ssMaterial(mat.diffuse, mat.metallic, mat.roughness));
	//	//matNames.push_back(RESOURCEMANAGER.getMaterial(i).name);
	//}

	//idk
	
	this->createInfo = createInfo;
	this->renderPass = createInfo.renderPass;
	vertexBuffer.device = createInfo.device->logicalDevice;
	indexBuffer.device = createInfo.device->logicalDevice;

#if defined(__ANDROID__)		
	if (vks::android::screenDensity >= ACONFIGURATION_DENSITY_XXHIGH) {
		scale = 3.5f;
	}
	else if (vks::android::screenDensity >= ACONFIGURATION_DENSITY_XHIGH) {
		scale = 2.5f;
	}
	else if (vks::android::screenDensity >= ACONFIGURATION_DENSITY_HIGH) {
		scale = 2.0f;
	};
#endif

	// Init ImGui
	// Color scheme
	ImGui::CreateContext();
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	// Dimensions
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)(createInfo.width), (float)(createInfo.height));
	io.FontGlobalScale = scale;



	cmdBuffers.resize(createInfo.framebuffers.size());
	prepareResources();
	if (createInfo.renderPass == VK_NULL_HANDLE) {
		prepareRenderPass();
	}
	preparePipeline();

	updateOverlay();

	findActiveCamera();
}
void EngineUISystem::initialize()
{
	editorMapper.init(*world);
	gcMapper.init(*world);

}
void EngineUISystem::processEntity(artemis::Entity & e)
{
	GlobalController* gc = gcMapper.get(e);
	glm::vec3 tempAxis = glm::vec3(0.f);
	for (int i = 0; i < NUM_GLOBAL_BUTTONS; ++i) {
		int action = INPUT.keys[gc->buttons[i].key];
		gc->buttons[i].action = action;
		if (action == GLFW_PRESS) {
			if (i == 6) { e.removeComponent<EditorComponent>(); 
			RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
			rs->togglePlayMode(true);
			ApplicationComponent* ac = (ApplicationComponent*)world->getSingleton()->getComponent<ApplicationComponent>();
			ac->state = AppState::Play;
			e.addComponent(new GameComponent());
			e.refresh();
			}
			if (i == 7) WINDOW.toggleMaximized();
			if (i == 8) glfwSetWindowShouldClose(WINDOW.getWindow(), 1);
		}
		if (action >= GLFW_PRESS) {
			gc->buttons[i].time += INPUT.deltaTime;
			if (i < 3)
				tempAxis[i] += 1.f;
			else if (i < 6)
				tempAxis[i - 3] -= 1.f;
		}
		else if (action == GLFW_RELEASE) {
			gc->buttons[i].time = 0.f;
		}
	}
	gc->axis = tempAxis;
}
void EngineUISystem::added(artemis::Entity & e)
{
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
	rs->togglePlayMode(false);
	ApplicationComponent* ac = (ApplicationComponent*)world->getSingleton()->getComponent<ApplicationComponent>();
	ac->state = AppState::Editor;
}
void EngineUISystem::removed(artemis::Entity & e)
{
	if (!world->getShutdown()) {
		RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
		rs->togglePlayMode(true);
		ApplicationComponent* ac = (ApplicationComponent*)world->getSingleton()->getComponent<ApplicationComponent>();
		ac->state = AppState::Play;
		ac->transition = true;
	}
}

void EngineUISystem::CleanUp() {
	vertexBuffer.destroy();
	indexBuffer.destroy();
	vkDestroyImageView(createInfo.device->logicalDevice, fontView, nullptr);
	vkDestroyImage(createInfo.device->logicalDevice, fontImage, nullptr);
	vkFreeMemory(createInfo.device->logicalDevice, fontMemory, nullptr);
	vkDestroySampler(createInfo.device->logicalDevice, sampler, nullptr);
	vkDestroyDescriptorSetLayout(createInfo.device->logicalDevice, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(createInfo.device->logicalDevice, descriptorPool, nullptr);
	vkDestroyPipelineLayout(createInfo.device->logicalDevice, pipelineLayout, nullptr);
	vkDestroyPipelineCache(createInfo.device->logicalDevice, pipelineCache, nullptr);
	vkDestroyPipeline(createInfo.device->logicalDevice, pipeline, nullptr);
	if (createInfo.renderPass == VK_NULL_HANDLE) {
		vkDestroyRenderPass(createInfo.device->logicalDevice, renderPass, nullptr);
	}
	vkFreeCommandBuffers(createInfo.device->logicalDevice, commandPool, static_cast<uint32_t>(cmdBuffers.size()), cmdBuffers.data());
	vkDestroyCommandPool(createInfo.device->logicalDevice, commandPool, nullptr);
	vkDestroyFence(createInfo.device->logicalDevice, fence, nullptr);
	cleaned = true;
}

/** Prepare all vulkan resources required to render the UI overlay */
void EngineUISystem::prepareResources()
{
	ImGuiIO& io = ImGui::GetIO();

	// Create font texture
	unsigned char* fontData;
	int texWidth, texHeight;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

	// Create target image for copy
	createInfo.device->createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fontImage, fontMemory);
	fontView = createInfo.device->createImageView(fontImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	// Staging buffers for font data upload
	vks::Buffer stagingBuffer;
	stagingBuffer.device = createInfo.device->logicalDevice;
	createInfo.device->createBuffer(uploadSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingBuffer.buffer,stagingBuffer.memory);
	
	stagingBuffer.map();
	memcpy(stagingBuffer.mapped, fontData, uploadSize);
	stagingBuffer.unmap();

	//createInfo.device->copyBufferToImage(stagingBuffer.buffer, fontImage, texWidth, texHeight);
	//Copy;
	VkCommandBuffer copyCmd = createInfo.device->beginSingleTimeCommands();
	VkImageSubresourceRange srr = {};
	srr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	srr.baseMipLevel = 0;
	srr.levelCount = 1;
	srr.layerCount = 1;
	VkImageMemoryBarrier imb = vks::initializers::imageMemoryBarrier();
	imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imb.image = fontImage;
	imb.subresourceRange = srr;
	imb.srcAccessMask = 0;
	imb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;	
	vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imb);

	VkBufferImageCopy bcr = {};
	bcr.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bcr.imageSubresource.layerCount = 1;
	bcr.imageExtent.width = texWidth;
	bcr.imageExtent.height = texHeight;
	bcr.imageExtent.depth = 1;
	vkCmdCopyBufferToImage(copyCmd, stagingBuffer.buffer, fontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bcr);
	
	//Prepare for shader read
	VkImageSubresourceRange srr2 = {};
	srr2.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	srr2.baseMipLevel = 0;
	srr2.levelCount = 1;
	srr2.layerCount = 1;
	VkImageMemoryBarrier imb2 = vks::initializers::imageMemoryBarrier();
	imb2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imb2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imb2.image = fontImage;
	imb2.subresourceRange = srr2;
	imb2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imb2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imb2);

	createInfo.device->endSingleTimeCommands(copyCmd); //flushcommandbuffer

	stagingBuffer.destroy();

	// Font texture Sampler
	VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECKRESULT(vkCreateSampler(createInfo.device->logicalDevice, &samplerInfo, nullptr, &sampler), "CREATE UI SAMPLER");

	// Command buffer
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = createInfo.device->qFams.graphicsFamily;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECKRESULT(vkCreateCommandPool(createInfo.device->logicalDevice, &cmdPoolInfo, nullptr, &commandPool), "CREATE UI COMMAND POOL");

	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(cmdBuffers.size()));
	VK_CHECKRESULT(vkAllocateCommandBuffers(createInfo.device->logicalDevice, &cmdBufAllocateInfo, cmdBuffers.data()), "ALLOCATE UI COMMADN BUFFERS");

	// Descriptor pool
	std::vector<VkDescriptorPoolSize> poolSizes = {
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
	VK_CHECKRESULT(vkCreateDescriptorPool(createInfo.device->logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool), "CREATE UI DESCRIPTOR POOL");

	// Descriptor set layout
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
	};
	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECKRESULT(vkCreateDescriptorSetLayout(createInfo.device->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout), "CREATE UI DESCRIPTOR SET LAYOUT");

	// Descriptor set
	VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
	VK_CHECKRESULT(vkAllocateDescriptorSets(createInfo.device->logicalDevice, &allocInfo, &descriptorSet), "ALLOCATE UI DESCRIPTOR SETS");
	VkDescriptorImageInfo fontDescriptor = vks::initializers::descriptorImageInfo(
		sampler,
		fontView,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);
	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
	};
	vkUpdateDescriptorSets(createInfo.device->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

	// Pipeline cache
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECKRESULT(vkCreatePipelineCache(createInfo.device->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache), "CREATE UI PIPELINE CACHE");

	// Pipeline layout
	// Push constants for UI rendering parameters
	VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	VK_CHECKRESULT(vkCreatePipelineLayout(createInfo.device->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), "CREATE UI PIPELINE LAYOUT");

	// Command buffer execution fence
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo();
	VK_CHECKRESULT(vkCreateFence(createInfo.device->logicalDevice, &fenceCreateInfo, nullptr, &fence), "CREATE UI FENCE");
}

/** Prepare a separate pipeline for the UI overlay rendering decoupled from the main application */
void EngineUISystem::preparePipeline()
{
	// Setup graphics pipeline for UI rendering
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

	// Enable blending
	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.blendEnable = VK_TRUE;
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	std::vector<VkPipelineColorBlendAttachmentState> blendStates(createInfo.attachmentCount);
	for (uint32_t i = 0; i < createInfo.attachmentCount; i++) {
		blendStates[i].blendEnable = VK_TRUE;
		blendStates[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendStates[i].colorBlendOp = VK_BLEND_OP_ADD;
		blendStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
	}

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		vks::initializers::pipelineColorBlendStateCreateInfo(static_cast<uint32_t>(blendStates.size()), blendStates.data());

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState =
		vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		vks::initializers::pipelineMultisampleStateCreateInfo(createInfo.rasterizationSamples);

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState =
		vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);

	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(createInfo.shaders.size());
	pipelineCreateInfo.pStages = createInfo.shaders.data();

	// Vertex bindings an attributes based on ImGui vertex definition
	std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
		vks::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
	};
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
		vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
		vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	// Location 1: UV
		vks::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
	};
	VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
	vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

	pipelineCreateInfo.pVertexInputState = &vertexInputState;

	VK_CHECKRESULT(vkCreateGraphicsPipelines(createInfo.device->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline), "CREATE UI GRAPHICS PIPELINE");
}

/** Prepare a separate render pass for rendering the UI as an overlay */
void EngineUISystem::prepareRenderPass()
{
	VkAttachmentDescription attachments[2] = {};

	// Color attachment
	attachments[0].format = createInfo.colorformat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Depth attachment
	attachments[1].format = createInfo.depthformat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDependency subpassDependencies[2] = {};

	// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Transition from initial to final
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.flags = 0;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = NULL;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pResolveAttachments = NULL;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = subpassDependencies;

	VK_CHECKRESULT(vkCreateRenderPass(createInfo.device->logicalDevice, &renderPassInfo, nullptr, &renderPass), "CREATE UI RENDER PASS");
}

/** Update the command buffers to reflect UI changes */
void EngineUISystem::updateCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.extent.width = createInfo.width;
	renderPassBeginInfo.renderArea.extent.height = createInfo.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(createInfo.clearValues.size());
	renderPassBeginInfo.pClearValues = createInfo.clearValues.data();

	ImGuiIO& io = ImGui::GetIO();

	for (size_t i = 0; i < cmdBuffers.size(); ++i) {
		renderPassBeginInfo.framebuffer = createInfo.framebuffers[i];

		VK_CHECKRESULT(vkBeginCommandBuffer(cmdBuffers[i], &cmdBufInfo), "BEGIN UI COMMAND BUFFER");
		/*
		if (vks::debugmarker::active) {
			vks::debugmarker::beginRegion(cmdBuffers[i], "UI overlay", glm::vec4(1.0f, 0.94f, 0.3f, 1.0f));
		}*/

		vkCmdBeginRenderPass(cmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmdBuffers[i], 0, 1, &vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(cmdBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

		VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
		vkCmdSetViewport(cmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D((int32_t)ImGui::GetIO().DisplaySize.x, (int32_t)ImGui::GetIO().DisplaySize.y, 0, 0);
		vkCmdSetScissor(cmdBuffers[i], 0, 1, &scissor);

		// UI scale and translate via push constants
		pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		pushConstBlock.translate = glm::vec2(-1.0f);
		vkCmdPushConstants(cmdBuffers[i], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

		// Render commands
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;
		for (int32_t j = 0; j < imDrawData->CmdListsCount; j++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[j];
			for (int32_t k = 0; k < cmd_list->CmdBuffer.Size; k++) {
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[k];
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				vkCmdSetScissor(cmdBuffers[i], 0, 1, &scissorRect);
				vkCmdDrawIndexed(cmdBuffers[i], pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}

		// Add empty subpasses if requested
		if (createInfo.subpassCount > 1) {
			for (uint32_t j = 1; j < createInfo.subpassCount; j++) {
				vkCmdNextSubpass(cmdBuffers[i], VK_SUBPASS_CONTENTS_INLINE);
			}
		}

		vkCmdEndRenderPass(cmdBuffers[i]);
		/*
		if (vks::debugmarker::active) {
			vks::debugmarker::endRegion(cmdBuffers[i]);
		}*/

		VK_CHECKRESULT(vkEndCommandBuffer(cmdBuffers[i]), "END UI COMMAND BUFFER");
		visible = true;
	}
}

/** Update vertex and index buffer containing the imGui elements when required */
void EngineUISystem::update()
{
	ImDrawData* imDrawData = ImGui::GetDrawData();
	bool updateCmdBuffers = false;

	if (!imDrawData) { return; };

	// Note: Alignment is done inside buffer creation
	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
	if (vertexBufferSize == 0 || indexBufferSize == 0)
		return;

	// Update buffers only if vertex or index count has been changed compared to current buffer size

	// Vertex buffer
	if ((vertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
		vertexBuffer.unmap();
		vertexBuffer.destroy();
		createInfo.device->createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer.buffer, vertexBuffer.memory);
		vertexCount = imDrawData->TotalVtxCount;
		vertexBuffer.unmap();
		vertexBuffer.map();
		updateCmdBuffers = true;
	}

	// Index buffer
	VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
	if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
		indexBuffer.unmap();
		indexBuffer.destroy();
		createInfo.device->createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBuffer.buffer, indexBuffer.memory);
		indexCount = imDrawData->TotalIdxCount;
		indexBuffer.map();
		updateCmdBuffers = true;
	}

	// Upload data
	ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
	ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.mapped;

	for (int n = 0; n < imDrawData->CmdListsCount; n++) {
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;
	}

	// Flush to make writes visible to GPU
	vertexBuffer.flush();
	indexBuffer.flush();

	if (updateCmdBuffers) {
		updateCommandBuffers();
	}
}

void EngineUISystem::resize(uint32_t width, uint32_t height, std::vector<VkFramebuffer> framebuffers)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)(width), (float)(height));
	createInfo.width = width;
	createInfo.height = height;
	createInfo.framebuffers = framebuffers;
	updateCommandBuffers();
}

/** Submit the overlay command buffers to a queue */
void EngineUISystem::submit(VkQueue queue, uint32_t bufferindex, VkSubmitInfo submitInfo)
{
	if (!visible) {
		return;
	}


	submitInfo.pCommandBuffers = &cmdBuffers[bufferindex];
	submitInfo.commandBufferCount = 1;

	VK_CHECKRESULT(vkQueueSubmit(queue, 1, &submitInfo, fence),"SUBMIT UI QUEUE");

	

	VK_CHECKRESULT(vkWaitForFences(createInfo.device->logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX), "WAIT FOR UI FENCES");
	VK_CHECKRESULT(vkResetFences(createInfo.device->logicalDevice, 1, &fence), "RESET UI FENCES");
}

//void EngineUISystem::addParentEntity(artemis::Entity * e, std::string name)
//{
//	parents.push_back((NodeComponent*)e->getComponent<NodeComponent>());
//	if (activeNode == nullptr)
//		activeNode = parents[0];
//	//parentNames.push_back(name);
//}

#pragma endregion

#pragma region Callbacks

bool EngineUISystem::header(const char *caption)
{
	return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
}

bool EngineUISystem::checkBox(const char *caption, bool *value)
{
	return ImGui::Checkbox(caption, value);
}

bool EngineUISystem::checkBox(const char *caption, int32_t *value)
{
	bool val = (*value == 1);
	bool res = ImGui::Checkbox(caption, &val);
	*value = val;
	return res;
}

bool EngineUISystem::inputFloat(const char *caption, float *value, float step, uint32_t precision)
{
	return ImGui::InputFloat(caption, value, step, step * 10.0f, precision);
}

bool EngineUISystem::sliderFloat(const char* caption, float* value, float min, float max)
{
	return ImGui::SliderFloat(caption, value, min, max);
}

bool EngineUISystem::sliderInt(const char* caption, int32_t* value, int32_t min, int32_t max)
{
	return ImGui::SliderInt(caption, value, min, max);
}

bool EngineUISystem::comboBox(const char *caption, int32_t *itemindex, std::vector<std::string> items)
{
	if (items.empty()) {
		return false;
	}
	std::vector<const char*> charitems;
	charitems.reserve(items.size());
	for (size_t i = 0; i < items.size(); i++) {
		charitems.push_back(items[i].c_str());
	}
	uint32_t itemCount = static_cast<uint32_t>(charitems.size());
	return ImGui::Combo(caption, itemindex, &charitems[0], itemCount, itemCount);
	
}

bool EngineUISystem::listBox(const char * caption, int32_t * itemindex, std::vector<std::string> items)
{
	if (items.empty()) {
		return false;
	}
	std::vector<const char*> charitems;
	charitems.reserve(items.size());
	for (size_t i = 0; i < items.size(); i++) {
		charitems.push_back(items[i].c_str());
	}
	uint32_t itemCount = static_cast<uint32_t>(charitems.size());
	return ImGui::ListBox(caption, itemindex, &charitems[0], itemCount, itemCount);
}

bool EngineUISystem::modelBox(const char * caption, int32_t * itemindex, std::vector<rModel> items)
{
	if (items.empty()) {
		return false;
	}
	std::vector<const char*> charitems;
	charitems.reserve(items.size());
	for (size_t i = 0; i < items.size(); i++) {
		charitems.push_back(items[i].name.c_str());
	}
	uint32_t itemCount = static_cast<uint32_t>(charitems.size());
	return ImGui::ListBox(caption, itemindex, &charitems[0], itemCount, itemCount);
}

bool EngineUISystem::matBox(const char * caption, int32_t * itemindex, std::vector<rMaterial> items)
{
	if (items.empty()) {
		return false;
	}
	std::vector<const char*> charitems;
	charitems.reserve(items.size());
	for (size_t i = 0; i < items.size(); i++) {
		charitems.push_back(items[i].name.c_str());
	}
	uint32_t itemCount = static_cast<uint32_t>(charitems.size());
	return ImGui::ListBox(caption, itemindex, &charitems[0], itemCount, itemCount);
}

bool EngineUISystem::animBox(const char * caption, int32_t * itemindex, std::vector<rSkeleton> items)
{
	if (items.empty()) {
		return false;
	}
	std::vector<const char*> charitems;
	charitems.reserve(items.size());
	for (size_t i = 0; i < items.size(); i++) {
		charitems.push_back(items[i].name.c_str());
	}
	uint32_t itemCount = static_cast<uint32_t>(charitems.size());
	return ImGui::ListBox(caption, itemindex, &charitems[0], itemCount, itemCount);

}

bool EngineUISystem::button(const char *caption)
{
	return ImGui::Button(caption);
}

void EngineUISystem::text(const char *formatstr, ...)
{
	va_list args;
	va_start(args, formatstr);
	ImGui::TextV(formatstr, args);
	va_end(args);
}

#pragma endregion

#pragma region Design
void EngineUISystem::topSection(float w, float h, bool* p_open)
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(w, h));// , ImGuiCond_FirstUseEver);
	static bool b_sphere = false;
	static bool b_box = false;
	static bool b_plane = false;
	static bool b_cylinder = false;

	if (!ImGui::Begin("Principia Engine", p_open, ImGuiWindowFlags_MenuBar | windowFlags)) {
		glfwSetWindowShouldClose(WINDOW.getWindow(), 1);
		ImGui::End();
	}
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("SaveScene")) SCENE.SaveScene("Level1/Scene1");//*p_open = false;
			if (ImGui::MenuItem("LoadScene")) { 
				//for(const auto & p fs::directory_iterator())
				SCENE.LoadScene("Level1/Scene1");
			};// *p_open = false;
			if (ImGui::MenuItem("SaveMaterials")) RESOURCEMANAGER.SaveMaterials();
			if (ImGui::MenuItem("Exit")) glfwSetWindowShouldClose(WINDOW.getWindow(), 1);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("Sphere")) b_sphere = true;
			if (ImGui::MenuItem("Box")) b_box = true;
			if (ImGui::MenuItem("Plane")) b_plane = true;
			if (ImGui::MenuItem("Cylinder")) b_cylinder = true;
			if (ImGui::MenuItem("Light")) *p_open = false;
			if (ImGui::MenuItem("New Scene")) *p_open = false;
			
			ImGui::EndMenu();
		}

		if (button("Translate")) eState = EditState::Translate;
		if (button("Rotate")) eState = EditState::Rotate;
		if (button("Scale")) eState = EditState::Scale;

		if (ImGui::BeginMenu("Options")) {
			ImGui::MenuItem("THERE ARE NO OPTIONS YOU FOOL!");
			if (ImGui::MenuItem("Close")) *p_open = false;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("ONLY GOD CAN HELP YOU!")) *p_open = false;
			ImGui::EndMenu();
		}
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("delta average %.3f ms/frame (%.1f FPS)", world->getDelta(), 1000.0f * world->getDelta());
		//ImGui::Text("Render average %.3f ms/frame (%.1f FPS)", INPUT.renderTime, 1000.0f / INPUT.renderTime);

		//ImGui::Text("Render average &.3f ms/frame (&.1f FPS)", 1000.f / rend)
		ImGui::EndMenuBar();
	}

	if (b_sphere) createSphere(b_sphere);
	if (b_box) createBox(b_box);
	if (b_cylinder) createCylinder(b_cylinder);
	if (b_plane) createPlane(b_plane);

	ImGui::End();
}

void EngineUISystem::bottomSection(float w, float h)
{
	ImGui::SetNextWindowPos(ImVec2(0, WINDOW.getHeight() - bottomHeight));
	ImGui::SetNextWindowSize(ImVec2(w, h));
	ImGui::Begin("Edit", nullptr, windowFlags);
	static bool b_editMat = false;
	if (numComponents > 0) {
		ImGui::Columns(numComponents);
		ImGui::Separator();
		if (activeNode->flags & COMPONENT_NODE) {
			editNode();
			ImGui::NextColumn();
		}
		if (activeNode->flags & COMPONENT_TRANSFORM) {
			editTransform();
			ImGui::NextColumn();
		}
		if (activeNode->flags & COMPONENT_MATERIAL) {
			if (button("Change Material")) {
				materialSelect();
				SCENE.updateObject(activeNode);
			}
			if (button("Edit Material")) {
				b_editMat = !b_editMat;
			}
			if (b_editMat) {
				editMaterial();
			}
			ImGui::NextColumn();
		}
		if (activeNode->flags & COMPONENT_SKINNED) {
			animationSelect();
			ImGui::NextColumn();
		}
		if (activeNode->flags & COMPONENT_CCONTROLLER) {
			editController();
			ImGui::NextColumn();
		}
		if (activeNode->flags & COMPONENT_LIGHT) {
			editLight();
			ImGui::NextColumn();
		}
	}

	ImGui::End();
}

void EngineUISystem::leftSection(float w, float h)
{
	ImGui::SetNextWindowPos(ImVec2(0, menuHeight));
	ImGui::SetNextWindowSize(ImVec2(w, h));// , ImGuiCond_FirstUseEver);
	ImGui::Begin("SceneGraph", nullptr, windowFlags);
	renderNodes(SCENE.parents, 0);
	ImGui::End();
}

void EngineUISystem::rightSection(float w, float h)
{
	ImGui::SetNextWindowPos(ImVec2(WINDOW.getWidth() *  0.8333333f, menuHeight));
	ImGui::SetNextWindowSize(ImVec2(w, h));// , ImGuiCond_FirstUseEver);
	ImGui::Begin("Resources", nullptr, windowFlags);

	//Models
	modelBox("Models", &modelIndex, RESOURCEMANAGER.getModels());
	static bool b_Model = false;
	if (ImGui::Button("Create Model"))
		b_Model = true;
	if(b_Model)createModel(b_Model);

	//materials
	matBox("Materials", &matIndex, RESOURCEMANAGER.getMaterials());
	static bool b_Material = false;
	if (ImGui::Button("Create Material"))
		b_Material = true;
	if (b_Material)createMaterial(b_Material);

	//animations
	animBox("Skinned Model", &skinnedIndex, RESOURCEMANAGER.getSkeletons());
	static bool b_Skinned = false;
	if (ImGui::Button("Create Skeleton"))
		b_Skinned = true;
	if (b_Skinned)
		createSkinnedModel(b_Skinned);

	ImGui::End();
}
#pragma endregion the main ui functions



void EngineUISystem::updateOverlay()
{
	//Timer timer("UI time: ");
	//if (!settings.overlay)
//	return;
	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2(WINDOW.getWidth(), WINDOW.getHeight());
	io.DeltaTime = abs(INPUT.deltaTime);

	bottomHeight = (int32_t)(io.DisplaySize.y * 0.3333333f) - menuHeight;

	io.MousePos = ImVec2(INPUT.mouse.x, INPUT.mouse.y);
	io.MouseDown[0] = INPUT.mouse.buttons[0] & (GLFW_REPEAT | GLFW_PRESS);
	io.MouseDown[1] = INPUT.mouse.buttons[1] & (GLFW_REPEAT | GLFW_PRESS);
	
	updateInput();

	ImGui::NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	
	leftSection(io.DisplaySize.x / 6, io.DisplaySize.y - bottomHeight - menuHeight);
	rightSection(io.DisplaySize.x / 6, io.DisplaySize.y - bottomHeight - menuHeight);
	bottomSection(io.DisplaySize.x, bottomHeight);
	topSection(io.DisplaySize.x, menuHeight, &menutest);
	ImGui::PopStyleVar();
	ImGui::Render();

	update();

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	if (mouseButtons.left) {
		mouseButtons.left = false;
	}
#endif
}

void EngineUISystem::updateMaterials()
{
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
	rs->updateMaterial(activeMaterial->matID);
	//rs->updateMaterials();
}

void EngineUISystem::updateActiveNode(NodeComponent * n)
{
	activeNode->clicked = false;
	activeNode = n;
	activeNode->clicked = true;
	componentVerify();
}

void EngineUISystem::updateInput()
{

	GlobalController* controller = (GlobalController*)world->getSingleton()->getComponent<GlobalController>();

	float moveSpeed = 10.f;
	if (INPUT.pressed && activeNode->flags & COMPONENT_TRANSFORM && eState == EditState::Translate) {
		//Input::Timer timer("Translate");
		activeTransform->local.position += controller->axis * moveSpeed * INPUT.deltaTime;
		//SCENE.ts->SQTTransform(activeNode, activeTransform->global);
		SCENE.ts->recursiveTransform(activeNode);// activeNode, activeTransform, glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f),true); //SCENE.ts->updateTransform(activeNode);
	}
	else if (INPUT.pressed && activeNode->flags & COMPONENT_TRANSFORM && eState == EditState::Scale) {
		activeTransform->local.scale += controller->axis * moveSpeed * INPUT.deltaTime;
		//SCENE.ts->SQTTransform(activeNode, activeTransform->global);
		SCENE.ts->recursiveTransform(activeNode);// activeNode, activeTransform, glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f), true); //SCENE.ts->updateTransform(activeNode);
	}
	else if (INPUT.pressed && activeNode->flags & COMPONENT_TRANSFORM && eState == EditState::Rotate) {
		//Input::Timer timer("Rotate");
		activeTransform->eulerRotation += controller->axis * moveSpeed * INPUT.deltaTime;

		//glm::mat4 rotationM;
		//rotationM = glm::rotate(rotationM, glm::radians(activeTransform->eulerRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//rotationM = glm::rotate(rotationM, glm::radians(activeTransform->eulerRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//rotationM = glm::rotate(rotationM, glm::radians(activeTransform->eulerRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//activeTransform->global.rotation = glm::toQuat(rotationM);
		//SCENE.ts->SQTTransform(activeNode, activeTransform->global);
		SCENE.ts->recursiveTransform(activeNode);// activeNode, activeTransform, glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f),true); //SCENE.ts->updateTransform(activeNode);
	}
}

bool EngineUISystem::renderNodes(std::vector<NodeComponent*>& nodes, int lvl)
{
	static bool b_MakeChild = false;
	if (lvl >= parentIndexes.size()) {
		parentIndexes.push_back(0);
	}

	//selection_mask[lvl] = (1 << 2);
	for (int i = 0; i < nodes.size(); ++i) {
		//if you have children make a tree
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (nodes[i]->clicked ? ImGuiTreeNodeFlags_Selected : 0);
		if (nodes[i]->children.size() > 0) {
			bool nodeopen = ImGui::TreeNodeEx((void*)(intptr_t)i, nodeFlags, nodes[i]->name.c_str(), i);
			if (ImGui::IsItemClicked()) {
				if (b_MakeChild) {
					b_MakeChild = false;
					SCENE.makeChild(activeNode, nodes[i], activeNode->parent == nullptr ? SCENE.parents : activeNode->parent->children);
					i--;
				}
				parentIndexes[lvl] = i;
				updateActiveNode(nodes[i]);
				if (ImGui::IsMouseDoubleClicked(0))
					moveCameraToNode();
			}
			if (nodeopen) {
				renderNodes(nodes[i]->children, lvl + 1);
				ImGui::TreePop();
			}
		}
		//No children, make a leaf
		else {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
			ImGui::TreeNodeEx((void*)(intptr_t)i, nodeFlags, nodes[i]->name.c_str(), i);
			if (ImGui::IsItemClicked()) {
				if (b_MakeChild) {
					b_MakeChild = false;
					SCENE.makeChild(activeNode, nodes[i], activeNode->parent == nullptr ? SCENE.parents : activeNode->parent->children);
					i--;
				}
				parentIndexes[lvl] = i;
				updateActiveNode(nodes[i]);
				if (ImGui::IsMouseDoubleClicked(0))
					moveCameraToNode();
			}
			
		}
		if (nodes[i] == activeNode) {
			if (ImGui::BeginPopupContextItem("Options")) {
				if (ImGui::Selectable("Copy")) SCENE.copyNode(activeNode, nullptr, SCENE.parents);
				if (ImGui::Selectable("Delete")) {// updateActiveNode(nodes[0]); 
					setActiveAsCamera();
					SCENE.deleteNode(nodes, i);
				}
				if (ImGui::Selectable("Rename")) {
					ImGui::InputText(nodes[i]->name.c_str(), (char*)nodes[i]->name.c_str(), IM_ARRAYSIZE(nodes[i]->name.c_str()));
				}
				if (ImGui::Selectable("Make Parent")) {
					SCENE.makeParent(activeNode);
				}
				if (ImGui::Selectable("Make Child")) {
					b_MakeChild = true;
				}
				if (ImGui::Selectable("Add Controller")) {
					SCENE.insertController(activeNode);
					updateActiveNode(activeNode);
				}
				if (ImGui::Selectable("Add Rigid Body")) {
					SCENE.insertRigidBody(activeNode);
					updateActiveNode(activeNode);
				}
				if (ImGui::Selectable("Add Collision")) {
					//activeNode->data->addComponent(new CollisionComponent());
					activeNode->flags |= COMPONENT_COLIDER;
					activeNode->data->refresh();
				}
				/*if (ImGui::Selectable("Make Spring")) {
					activeNode->data->addComponent(new SpringComponent(glm::vec3(0.f, 1.f, 0.f), 10.f));
					activeNode->flags |= COMPONENT_SPRING;
					activeNode->data->refresh();
				}
				if (ImGui::Selectable("Make Button")) {
					activeNode->data->addComponent(new ButtonComponent());
					activeNode->flags |= COMPONENT_BUTTON;
					activeNode->data->addComponent(new CollisionComponent());
					activeNode->flags |= COMPONENT_COLIDER;
					activeNode->data->refresh();


					RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
					ButtonSystem* bs = (ButtonSystem*)world->getSystemManager()->getSystem<ButtonSystem>();

					bs->change(*activeNode->data);

				}*/
				ImGui::EndPopup();
			}
		}

	}
	return true;
}

void EngineUISystem::componentVerify()
{
	numComponents = 0;

	if (activeNode->flags & COMPONENT_NODE)numComponents++;
	if (activeNode->flags & COMPONENT_TRANSFORM) {
		numComponents++;
		activeTransform = (TransformComponent*)activeNode->data->getComponent<TransformComponent>();
	}
	if (activeNode->flags & COMPONENT_MATERIAL) {
		numComponents++;
		activeMaterial = (MaterialComponent*)activeNode->data->getComponent<MaterialComponent>();
	}
	if (activeNode->flags & COMPONENT_SKINNED) {
		numComponents++;
		activeAnimation = (AnimationComponent*)activeNode->data->getComponent<AnimationComponent>();
		numAnims = RESOURCEMANAGER.getSkeletonID(activeAnimation->skeleton.id).animations.size();
		animIndex = activeAnimation->animIndex;
	}
	if (activeNode->flags & COMPONENT_CCONTROLLER) {
		numComponents++;
		activeController = (CharacterComponent*)activeNode->data->getComponent<CharacterComponent>();
	}
	if (activeNode->flags & COMPONENT_LIGHT) {
		numComponents++;
		activeLight = (LightComponent*)activeNode->data->getComponent<LightComponent>();
	}
}

void EngineUISystem::editNode()
{
	static char str0[128] = "Name";// (char*)activeNode->name.c_str();
	if (ImGui::InputText("Name", str0, IM_ARRAYSIZE(str0)))
		activeNode->name = str0;
}

void EngineUISystem::editTransform()
{
	float* pos[3] = { &activeTransform->local.position.x, &activeTransform->local.position.y, &activeTransform->local.position.z };
	float* rot[3] = { &activeTransform->eulerRotation.x, &activeTransform->eulerRotation.y, &activeTransform->eulerRotation.z };
	float* sca[3] = { &activeTransform->local.scale.x, &activeTransform->local.scale.y, &activeTransform->local.scale.z };

	ImGui::Text("Position");
	if (ImGui::DragFloat3("p", *pos, 0.01f))
		SCENE.ts->recursiveTransform(activeNode);// activeNode, activeTransform, glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f), true); //SCENE.ts->updateTransform(activeNode);
	ImGui::Text("Rotation");
	if (ImGui::DragFloat3("r", *rot, 1.f)) {
		//glm::mat4 rotationM;
		//rotationM = glm::rotate(rotationM, glm::radians(activeTransform->eulerRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//rotationM = glm::rotate(rotationM, glm::radians(activeTransform->eulerRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//rotationM = glm::rotate(rotationM, glm::radians(activeTransform->eulerRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//activeTransform->local.rotation = glm::toQuat(rotationM);
		SCENE.ts->recursiveTransform(activeNode);// activeNode, activeTransform, glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f), true); //SCENE.ts->updateTransform(activeNode);
	}
	ImGui::Text("Scale");
	if (ImGui::DragFloat3("s", *sca, 0.01f))
		SCENE.ts->recursiveTransform(activeNode);// activeNode, activeTransform, glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f), true); //SCENE.ts->updateTransform(activeNode);
}

void EngineUISystem::editGeometry()
{
}

void EngineUISystem::editMaterial()
{
	//float diff[3] = { activeMaterial->diffuse->r, activeMaterial->diffuse->g, activeMaterial->diffuse->b };
	//if (ImGui::ColorEdit3("Diffuse", (float*)activeMaterial->diffuse) ||// diff);
	//	sliderFloat("Metallic", activeMaterial->metallic, 0.f, 1.f) ||
	//	sliderFloat("Roughness", activeMaterial->roughness, 0.f, 1.f)) {
	//
	//	updateMaterials();
	//}
	rMaterial& mat = RESOURCEMANAGER.getMaterial(activeMaterial->matID);
	//float* diff[3] = { &mat.diffuse.r, &mat.diffuse.g, &mat.diffuse.b };
	//static int textureID = 0;
	if (ImGui::ColorEdit3("Diffuse", (float*)&mat.diffuse)		 ||
		sliderFloat("Reflective", &mat.reflective, 0.f, 1.f)	 ||
		sliderFloat("Roughness", &mat.roughness, 0.f, 1.f)		 ||
		sliderFloat("Transparency", &mat.transparency, 0.f, 1.f) ||
		sliderFloat("RefractiveIndex", &mat.refractiveIndex, 1.f, 2.f) ||
		sliderInt("TextureID", &mat.textureID, 0, MAXTEXTURES - 1)) {

		updateMaterials();
	}
}
void EngineUISystem::editLight()
{
	ImGui::SliderFloat("Intensity", &activeLight->intensity, 0.f, 1000.f, "%.3f", 1.f);

	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();

	ssLight& light = rs->getLight(activeLight->id);
	light.intensity = activeLight->intensity;
	rs->setRenderUpdate(RenderSystem::UPDATE_LIGHT);
}

void EngineUISystem::editCamera()
{
}

void EngineUISystem::editController() {
	//static float* characterData[4] = { &activeController->data[0], &activeController->data[1], &activeController->data[2], &activeController->data[3] };
	static float* characterData[4] = { &activeController->speed, &activeController->maxSpeed, &activeController->jumpSpeed, &activeController->maxJumpHeight };

	ImGui::SliderFloat("Speed", characterData[0], 0.f, 100.f, "%.3f", 1.f);
	ImGui::SliderFloat("Jump Speed", characterData[2], 0.f, 20.f, "%.4f", 1.f);
}

void EngineUISystem::createModel(bool & p_create)
{
	ImGui::OpenPopup("Create Model");
	if (ImGui::BeginPopupModal("Create Model"))
	{
		static char str[128] = "New Model!";
		static float* position[3] = { &pos.x, &pos.y, &pos.z };
		static float* rotation[3] = { &rot.x, &rot.y, &rot.z };
		static float* scale[3] = { &sca.x, &sca.y, &sca.z };

		ImGui::Text("Edit the Transform of yo Model y0");
		ImGui::InputText("Enter Name", str, IM_ARRAYSIZE(str));

		ImGui::Text("Position");
		ImGui::DragFloat3("Position", *position, 0.01f);

		ImGui::Text("Rotation");
		ImGui::DragFloat3("Rotation", *rotation, 0.01f);

		ImGui::Text("Scale");
		ImGui::DragFloat3("Scale", *scale, 0.001f);



		ImGui::Checkbox("Is Dynamic? ", &isDynamic);

		if (ImGui::Button("Create Model")) {
			SCENE.createModel(RESOURCEMANAGER.getModel(modelIndex), str, pos, rot, sca, isDynamic);
			p_create = false;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("Close")) {
			p_create = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void EngineUISystem::createSkinnedModel(bool & p_create)
{
	ImGui::OpenPopup("Create Skinned Model");
	if (ImGui::BeginPopupModal("Create Skinned Model"))
	{
		static char str[128] = "New Model!";
		static float* position[3] = { &pos.x, &pos.y, &pos.z };
		static float* rotation[3] = { &rot.x, &rot.y, &rot.z };
		static float* scale[3] = { &sca.x, &sca.y, &sca.z };

		ImGui::Text("Edit the Transform of yo Model y0");
		ImGui::InputText("Enter Name", str, IM_ARRAYSIZE(str));

		ImGui::Text("Position");
		ImGui::DragFloat3("Position", *position, 0.01f);

		ImGui::Text("Rotation");
		ImGui::DragFloat3("Rotation", *rotation, 0.01f);

		ImGui::Text("Scale");
		ImGui::DragFloat3("Scale", *scale, 0.001f);



		ImGui::Checkbox("Is Dynamic? ", &isDynamic);

		if (ImGui::Button("Create Model")) {
			SCENE.createSkinnedModel(RESOURCEMANAGER.getSkeleton(modelIndex), str, pos, rot, sca, isDynamic);
			p_create = false;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("Close")) {
			p_create = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void EngineUISystem::createSphere(bool& p_create)
{
	ImGui::OpenPopup("Create Sphere");
	if (ImGui::BeginPopupModal("Create Sphere"))
	{
		static char str[128] = "New Sphere!";
		static float* position[3] = { &pos.x, &pos.y, &pos.z };
		static float radius = 1.0f;
		ImGui::Text("Edit the Transform of yo Sphere y0");
		ImGui::InputText("Enter Name", str, IM_ARRAYSIZE(str));

		ImGui::Text("Position");
		ImGui::DragFloat3("Position", *position, 0.01f);

		ImGui::Text("Radius");
		ImGui::DragFloat3("Radius", &radius, 0.01f);


		ImGui::Checkbox("Is Dynamic? ", &isDynamic);

		if (ImGui::Button("Create Sphere")) {
			//SCENE.createSphere(str, pos, radius, matIndex, isDynamic);
			SCENE.createShape(str, pos, glm::vec3(radius), matIndex, (int)ObjectType::SPHERE, isDynamic);
			ImGui::CloseCurrentPopup();
			p_create = false;
		}
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
			p_create = false;
		}
		ImGui::EndPopup();
	}
}
void EngineUISystem::createBox(bool & p_create)
{
	ImGui::OpenPopup("Create Box");
	if (ImGui::BeginPopupModal("Create Box"))
	{
		static char str[128] = "New Box!";
		static float* position[3] = { &pos.x, &pos.y, &pos.z };
		static float* scale[3] = {&sca.x, &sca.y, &sca.z };

		ImGui::Text("Edit the Transform of yo Box y0");
		ImGui::InputText("Enter Name", str, IM_ARRAYSIZE(str));

		ImGui::Text("Center");
		ImGui::DragFloat3("Center", *position, 0.01f);

		ImGui::Text("Extents");
		ImGui::DragFloat3("Extents", *scale, 0.01f);

		ImGui::Checkbox("Is Dynamic? ", &isDynamic);

		if (ImGui::Button("Create Box")) {
			//SCENE.createBox(str, pos, sca, matIndex, isDynamic);
			SCENE.createShape(str, pos, sca, matIndex, (int)ObjectType::BOX, isDynamic);
			ImGui::CloseCurrentPopup();
			p_create = false;
		}
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
			p_create = false;
		}
		ImGui::EndPopup();
	}
}

void EngineUISystem::createPlane(bool & p_create)
{
	ImGui::OpenPopup("Create Plane");
	if (ImGui::BeginPopupModal("Create Plane"))
	{
		static char str[128] = "New Plane!";
		static float* normal[3] = { &rot.x, &rot.y, &rot.z };
		static float distance = 5.f;

		ImGui::Text("Edit the Transform of yo Plane y0");
		ImGui::InputText("Enter Name", str, IM_ARRAYSIZE(str));

		ImGui::Text("Normal");
		ImGui::DragFloat3("Normal", *normal, 0.1f);
		ImGui::Text("Distance");
		ImGui::DragFloat("Distance", &distance, 0.01f);
		ImGui::Checkbox("Dynamic?", &isDynamic);

		if (ImGui::Button("Create Plane")) {
			//SCENE.createPlane(str, rot, distance, matIndex, isDynamic);
			SCENE.createShape(str, pos, rot, matIndex, (int)ObjectType::PLANE, isDynamic);
			ImGui::CloseCurrentPopup();
			p_create = false;
		}
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
			p_create = false;
		}
		ImGui::EndPopup();
	}
}

void EngineUISystem::createCylinder(bool & p_create)
{
	ImGui::OpenPopup("Create Cylinder");
	if (ImGui::BeginPopupModal("Create Cylinder"))
	{
		static char str[128] = "New Cylinder!";
		static float* position[3] = { &pos.x, &pos.y, &pos.z };
		static float* scale[3] = {&sca.x, &sca.y, &sca.z};
		ImGui::Text("Edit the Transform of yo Cylinder y0");
		ImGui::InputText("Enter Name", str, IM_ARRAYSIZE(str));

		ImGui::Text("Position");
		ImGui::DragFloat3("Position", *position, 0.01f);

		ImGui::Text("Scale");
		ImGui::DragFloat3("Scale", *scale, 0.01f);

		ImGui::Checkbox("Is Dynamic? ", &isDynamic);

		if (ImGui::Button("Create Cylinder")) {
			//SCENE.createCylinder(str, pos, sca, matIndex, isDynamic);
			SCENE.createShape(str, pos, sca, matIndex, (int)ObjectType::CYLINDER, isDynamic);
			ImGui::CloseCurrentPopup();
			p_create = false;
		}
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
			p_create = false;
		}
		ImGui::EndPopup();
	}
}

void EngineUISystem::createMaterial(bool & p_create)
{
	ImGui::OpenPopup("Create Material");
	if (ImGui::BeginPopupModal("Create Material"))
	{
		static char str2[128] = "New Material!";
		static float mdiff[3] = { 1.f, 1.f, 1.f };
		static float reflective = 1.0f;
		static float roughness = 1.0f;
		static float transparency = 0.0f;
		static float refractiveIndex = 0.0f;

		ImGui::InputText("New Material", str2, IM_ARRAYSIZE(str2));
		ImGui::ColorEdit3("Diffuse", mdiff);
		sliderFloat("Reflectiveness", &reflective, 0.f, 1.f);
		sliderFloat("Roughness", &roughness, 0.f, 1.f);
		sliderFloat("Transparency", &transparency, 0.f, 1.f);
		sliderFloat("RefractiveIndex", &refractiveIndex, 1.f, 2.f);

		if (ImGui::Button("Create Material")) {
			std::string s = str2;
			glm::vec3 diffuse = glm::vec3(mdiff[0], mdiff[1], mdiff[2]);
			RESOURCEMANAGER.addMaterial(s, diffuse, reflective, roughness, transparency, refractiveIndex);
			RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();
			rs->addMaterial(diffuse, reflective, roughness, transparency, refractiveIndex);

			p_create = false;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("Close")) {
			p_create = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void EngineUISystem::materialSelect()
{
	activeMaterial->matID = matIndex;
	activeMaterial->matUnqiueID = RESOURCEMANAGER.getMaterial(matIndex).uniqueID;
	RenderSystem* rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();

	if (activeNode->flags & COMPONENT_PRIMITIVE) {
		PrimitiveComponent* objComp = (PrimitiveComponent*)activeNode->data->getComponent<PrimitiveComponent>();
		rs->getObject(objComp->objIndex).matId = matIndex;
		rs->setRenderUpdate(RenderSystem::UPDATE_OBJECT);
	}

}

void EngineUISystem::animationSelect()
{
	//ImGui::DragInt("Animation Index", &animIndex, 0.01f, 0, numAnims - 1);
	ImGui::SliderInt("Animation Index", &animIndex, 0, numAnims - 1, "AniIndex: %d ");
	if (activeAnimation->animIndex != animIndex) {
		//Refresh if there's a change
		activeAnimation->animIndex = animIndex;
		activeAnimation->time = 0;
		for (int i = 0; i < activeAnimation->channels.size(); ++i) {
			activeAnimation->channels[i].key_current = 0;
			activeAnimation->channels[i].key_next = 1;
		}
	}
	ImGui::Checkbox("Anim On", &activeAnimation->on);
}

void EngineUISystem::findActiveCamera()
{
	for each (NodeComponent* node in SCENE.parents)
	{
		CameraComponent* cam = (CameraComponent*)node->data->getComponent<CameraComponent>();
		if (cam != nullptr) {
			activeCamera = node;
			return;
		}
	}
}

void EngineUISystem::setActiveAsCamera()
{
	if (activeNode != nullptr) {
		activeNode->clicked = false;
	}
	if (SCENE.parents[0]->flags & COMPONENT_CAMERA)
		activeNode = SCENE.parents[0];
	else {
		for each (NodeComponent* node in SCENE.parents)
		{
			if (node->flags & COMPONENT_CAMERA)
				activeNode = node;
		}
	}
	activeTransform = (TransformComponent*)activeNode->data->getComponent<TransformComponent>();
	activeNode->clicked = true;
	componentVerify();
}

void EngineUISystem::moveCameraToNode()
{
	//Makes it so you move the camera to the node and hovers the camera to be like 1 or maybe 2 over it
	TransformComponent* cameraTransform = (TransformComponent*)activeCamera->data->getComponent<TransformComponent>();
	cameraTransform->local.position = activeTransform->local.position;
	cameraTransform->local.position.y += 2.f + activeTransform->local.scale.y;
	SCENE.ts->recursiveTransform(activeCamera);

}

void EngineUISystem::refresh()
{
	pos = glm::vec3(0.f);
	rot = glm::vec3(0.f);
	sca = glm::vec3(1.f);
	//str = "New Mesh";
}
