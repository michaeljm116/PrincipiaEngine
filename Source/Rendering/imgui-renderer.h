#pragma once
#include "renderer.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Principia {

	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	class ImGuiRenderer {
	public:

		ImGuiRenderer(Renderer* r);
		~ImGuiRenderer();

		void initImGui();
		void destroyImGui();

		void start_draw(VkSubmitInfo* submitInfo, int image_index);
		void end_draw(VkSubmitInfo* submitInfo, int ii);

	private:
		Renderer* renderer;
		VkDescriptorPool descriptor_pool;
		VkRenderPass render_pass;
		std::vector<VkCommandBuffer> command_buffers;
		VkCommandPool command_pool;
		std::vector<VkFramebuffer> ui_framebuffers;
		std::vector<VkSemaphore> ui_semaphores;
		VkQueue copy_queue;

		std::array<VkClearValue, 2> clearValues = {};

		void create_descriptor_pool();
		void create_render_pass();
		void create_frame_buffers();
		void create_command_pool();
		void create_command_buffers();
	};

}