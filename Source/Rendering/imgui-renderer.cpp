#include "../pch.h"
#include "imgui-renderer.h"

namespace Principia {
	ImGuiRenderer::ImGuiRenderer(Renderer* r)
	{
		renderer = r;
	}
	ImGuiRenderer::~ImGuiRenderer()
	{
		destroyImGui();
	}
	void ImGuiRenderer::initImGui()
	{
		create_descriptor_pool();
		create_render_pass();
		create_frame_buffers();
		create_command_pool();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch


		ImGui_ImplGlfw_InitForVulkan(WINDOW.getWindow(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = renderer->vkDevice.instance;
		init_info.PhysicalDevice = renderer->vkDevice.physicalDevice;
		init_info.Device = renderer->vkDevice.logicalDevice;
		init_info.QueueFamily = renderer->vkDevice.qFams.graphicsFamily;
		init_info.Queue = renderer->graphicsQueue;
		init_info.PipelineCache = renderer->pipelineCache;
		init_info.DescriptorPool = descriptor_pool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 2;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = VK_NULL_HANDLE;
		init_info.CheckVkResultFn = check_vk_result;
		init_info.RenderPass = render_pass;
		ImGui_ImplVulkan_Init(&init_info);

		
		ui_semaphores.resize(ui_framebuffers.size());
		for (auto& ui_semaphore : ui_semaphores) {
			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VK_CHECKRESULT(vkCreateSemaphore(renderer->vkDevice.logicalDevice, &semaphoreInfo, nullptr, &ui_semaphore), " FAILED TO CREATE SEMAPHORE");
		}


		create_command_buffers();
		copy_queue = renderer->GetDeviceInfo().copyQueue;
	}
	void ImGuiRenderer::destroyImGui()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		for (int i = 0; i < ui_framebuffers.size(); ++i) {
			vkDestroySemaphore(renderer->vkDevice.logicalDevice, ui_semaphores[i], nullptr);
			vkDestroyFramebuffer(renderer->vkDevice.logicalDevice, ui_framebuffers[i], nullptr);
		}
		vkDestroyRenderPass(renderer->vkDevice.logicalDevice, render_pass, nullptr);
		vkDestroyDescriptorPool(renderer->vkDevice.logicalDevice, descriptor_pool, nullptr);
		vkFreeCommandBuffers(renderer->vkDevice.logicalDevice, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
		vkDestroyCommandPool(renderer->vkDevice.logicalDevice, command_pool, nullptr);
	}
	void ImGuiRenderer::start_draw(VkSubmitInfo* submitInfo, int image_index)
	{
		// First Wait for the compute to finish renderering
		submitInfo->waitSemaphoreCount = 1;
		submitInfo->pWaitSemaphores = submitInfo->pSignalSemaphores;
		submitInfo->signalSemaphoreCount = 1;
		submitInfo->pSignalSemaphores = &ui_semaphores[0];
		submitInfo->commandBufferCount = 1;
		submitInfo->pCommandBuffers = &command_buffers[image_index];

		// Now begin rendering imgui
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = render_pass;
		renderPassBeginInfo.renderArea.extent.width = renderer->swapChainExtent.width;
		renderPassBeginInfo.renderArea.extent.height = renderer->swapChainExtent.height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.framebuffer = ui_framebuffers[image_index];

		VK_CHECKRESULT(vkBeginCommandBuffer(command_buffers[image_index], &cmdBufInfo), "BEGIN UI COMMAND BUFFER");
		vkCmdBeginRenderPass(command_buffers[image_index], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void ImGuiRenderer::end_draw(VkSubmitInfo* submit_info, int ii) 
	{
		VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
		vkCmdSetViewport(command_buffers[ii], 0, 1, &viewport);
		VkRect2D scissor = vks::initializers::rect2D((int32_t)ImGui::GetIO().DisplaySize.x, (int32_t)ImGui::GetIO().DisplaySize.y, 0, 0);
		vkCmdSetScissor(command_buffers[ii], 0, 1, &scissor);

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffers[ii]);

		vkCmdEndRenderPass(command_buffers[ii]);
		VK_CHECKRESULT(vkEndCommandBuffer(command_buffers[ii]), "END UI COMMAND BUFFER");
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info->pWaitDstStageMask = waitStages;
		VK_CHECKRESULT(vkQueueSubmit(renderer->graphicsQueue, 1, submit_info, VK_NULL_HANDLE), "UI QUEUE SUBMIT");
	}
	void ImGuiRenderer::create_descriptor_pool()
	{
		//create the imgui descriptor pool
		VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		
		VK_CHECKRESULT(vkCreateDescriptorPool(renderer->vkDevice.logicalDevice, &pool_info, nullptr, &descriptor_pool), "CREATING IMGUI DESCRIPTOR POOL");
	}
	void ImGuiRenderer::create_render_pass()
	{
		VkAttachmentDescription attachments[2] = {};

		// Color attachment
		attachments[0].format = renderer->swapChainImageFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Depth attachment
		attachments[1].format = renderer->depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference  = {};
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

		VK_CHECKRESULT(vkCreateRenderPass(renderer->vkDevice.logicalDevice, &renderPassInfo, nullptr, &render_pass), "CREATE UI RENDER PASS");
	}
	void ImGuiRenderer::create_frame_buffers()
	{
		ui_framebuffers.resize(renderer->GetFrameBuffers().size());
		for (auto i = 0; i < renderer->swapChainImageViews.size(); ++i) {
			std::array<VkImageView, 2> attachments = {
				renderer->swapChainImageViews[i],
				renderer->depthImageView
			};

			VkFramebufferCreateInfo framebuffer_info = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = render_pass,
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(),
				.width = renderer->swapChainExtent.width,
				.height = renderer->swapChainExtent.height,
				.layers = 1
			};

			VK_CHECKRESULT(vkCreateFramebuffer(renderer->vkDevice.logicalDevice, &framebuffer_info, nullptr, &ui_framebuffers[i]), "Creating UI FrameBuffer");
		}
	}

	void ImGuiRenderer::create_command_pool()
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = renderer->vkDevice.qFams.graphicsFamily;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECKRESULT(vkCreateCommandPool(renderer->vkDevice.logicalDevice, &cmdPoolInfo, nullptr, &command_pool), "CREATE UI COMMAND POOL");
	}
	void ImGuiRenderer::create_command_buffers()
	{
		command_buffers.resize(renderer->GetFrameBuffers().size());
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(command_buffers.size()));
		VK_CHECKRESULT(vkAllocateCommandBuffers(renderer->vkDevice.logicalDevice, &cmdBufAllocateInfo, command_buffers.data()), "ALLOCATE UI COMMADN BUFFERS");

		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
		clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
		renderPassBeginInfo.renderPass = render_pass;
		renderPassBeginInfo.renderArea.extent.width = renderer->swapChainExtent.width;
		renderPassBeginInfo.renderArea.extent.height = renderer->swapChainExtent.height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		ImGuiIO& io = ImGui::GetIO();

		for (size_t i = 0; i < command_buffers.size(); ++i) {
			renderPassBeginInfo.framebuffer = ui_framebuffers[i];
			VK_CHECKRESULT(vkBeginCommandBuffer(command_buffers[i], &cmdBufInfo), "BEGIN UI COMMAND BUFFER");
			vkCmdBeginRenderPass(command_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			VkViewport viewport = vks::initializers::viewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);
			vkCmdSetViewport(command_buffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D((int32_t)ImGui::GetIO().DisplaySize.x, (int32_t)ImGui::GetIO().DisplaySize.y, 0, 0);
			vkCmdSetScissor(command_buffers[i], 0, 1, &scissor);
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffers[i]);
			vkCmdEndRenderPass(command_buffers[i]);
			VK_CHECKRESULT(vkEndCommandBuffer(command_buffers[i]), "END UI COMMAND BUFFER");
		}
	}
}