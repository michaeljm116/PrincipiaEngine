#pragma once
/*Renderbase Copyright (C) by Mike Murrell 2017
*/

#include "vulkanbase.h"
#include "../Utility/window.h"
#include "../ArtemisFrameWork/Artemis/Artemis.h"
#include<array>

namespace Principia {

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
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
		VulkanDevice vkDevice;

	protected:
		VkSurfaceKHR surface;

		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue computeQueue;

		VkRenderPass renderPass;
		VkPipelineCache	pipelineCache;

		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		std::vector<VkCommandBuffer> commandBuffers;
		VkCommandPool commandPool;

		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkSemaphore uiSemaphore;

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

		bool isDeviceSuitable(VkPhysicalDevice);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector <VkPresentModeKHR>);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);
		std::vector<const char*> getRequiredExtentions();

		void			setComputeQueueFamilyIndex();
	};

}