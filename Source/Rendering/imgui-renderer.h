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

		void DrawImGui(VkSubmitInfo* submitInfo, int image_index);

	private:
		Renderer* renderer;
		VkDescriptorPool descriptor_pool;
		VkRenderPass render_pass;
		std::vector<VkCommandBuffer> command_buffers;
		VkCommandPool command_pool;
		VkSemaphore ui_semaphore;
		VkQueue copy_queue;

		std::array<VkClearValue, 2> clearValues = {};

		void create_descriptor_pool();
		void create_render_pass();
		void create_command_pool();
		void create_command_buffers();
		void update_command_buffers(int ii);
	};

}