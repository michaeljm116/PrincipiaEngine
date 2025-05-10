#pragma once
/*Renderbase Copyright (C) by Mike Murrell 2017
*/

#include "vulkanbase.h"
#include "../Utility/window.h"
#include "../ArtemisFrameWork/Artemis/Artemis.h"
#include<array>
#include <GLFW/glfw3.h>

namespace Principia {

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats = {}; 
		std::vector<VkPresentModeKHR> presentModes = {};
	};

	struct VkDeviceInfo {
		VulkanDevice* device = nullptr;
		VkQueue copyQueue = VK_NULL_HANDLE;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> framebuffers;
		VkFormat colorFormat = VK_FORMAT_UNDEFINED;
		VkFormat depthFormat = VK_FORMAT_UNDEFINED;
		uint32_t width = 0;
		uint32_t height = 0;
	};

	class RenderBase {
	public:
		RenderBase() {};
		~RenderBase() {

		};

		void initWindow();
		void initVulkan();
		virtual void cleanup();
		virtual void cleanupSwapChain();
		virtual void recreateSwapChain();

		static void onWindowResized(GLFWwindow* window, int width, int height) {
			if (width == 0 || height == 0) return;

			RenderBase* app = reinterpret_cast<RenderBase*>(glfwGetWindowUserPointer(window));
			app->recreateSwapChain();
		}
		VulkanDevice vkDevice = {};

	public:
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkImage depthImage = VK_NULL_HANDLE;
		VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
		VkImageView depthImageView = VK_NULL_HANDLE;
		VkFormat depthFormat = VK_FORMAT_UNDEFINED;

		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		VkQueue computeQueue = VK_NULL_HANDLE;

		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkPipelineCache pipelineCache = VK_NULL_HANDLE;

		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D swapChainExtent = {};

		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		std::vector<VkCommandBuffer> commandBuffers;
		VkCommandPool commandPool = VK_NULL_HANDLE;

		VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;

	private:
		void createInstance();
		void createSurface();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createPipelineCache();
		void createDepthResources();
		void createFrameBuffers();
		void createCommandPool();
		void createSemaphores();
		void pickPhysicalDevice();
	protected:
		VkFormat findDepthFormat();
		VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);

		int isDeviceSuitable(VkPhysicalDevice);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector <VkPresentModeKHR>);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);
		std::vector<const char*> getRequiredExtentions();

		void			setComputeQueueFamilyIndex();
	};

}