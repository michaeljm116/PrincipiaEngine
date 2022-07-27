#pragma once
/*Texture class Copyright (C) by Mike Murrell 2017
honestly,,, i dont even know if i still use this class
nvm turns out i still use it
nvm turns out i still use it
*/
//#include "RenderHelplers.h"
#include "vulkanbase.h"
#include <assert.h>

namespace Principia {
	struct Texture
	{
	public:
		Texture();
		Texture(std::string p) : path(p) {};
		~Texture();

		VkImage image;
		VkImageView view;
		VkImageLayout imageLayout;
		VkDeviceMemory memory;
		VkSampler sampler;
		int width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;
		std::string path;
		VkDescriptorSet descriptor_set;
		//VulkanDevice* device;

		void destroy(VkDevice& device);
		VkResult CreateTexture(VulkanDevice& device);
		void updateDescriptor();
	};

	struct PrPixel {
		unsigned char r, g, b, a = 1;
		unsigned char& operator[](int i) {
			assert(i > -1 && i < 4);
			return *(&r + i);
		}
	};

	struct PrImage {
		unsigned int width, height, channels;
		std::vector<std::vector<PrPixel>> data;
		PrImage(int image_width, int image_height, int image_channels) : width(image_width), height(image_height), channels(image_channels) {
			data = std::vector(width, std::vector<PrPixel>(height));
		}
	};

}