#pragma once
/*Texture class Copyright (C) by Mike Murrell 2017
honestly,,, i dont even know if i still use this class
nvm turns out i still use it
nvm turns out i still use it
*/
//#include "RenderHelplers.h"
#include "vulkanbase.h"
#include <assert.h>
#include <stb_image.h>

namespace Principia {
	struct Texture
	{
	public:
		Texture();
		Texture(std::string p) : path(p) {};
		Texture(std::string p, VulkanDevice& device);
		~Texture();

		VkImage image = VkImage();
		VkImageView view = VkImageView();
		VkImageLayout imageLayout = VkImageLayout();
		VkDeviceMemory memory = VkDeviceMemory();
		VkSampler sampler = VkSampler();
		int width = 0, height = 0;
		uint32_t mipLevels = 0;
		uint32_t layerCount = 0;
		VkDescriptorImageInfo descriptor = VkDescriptorImageInfo();
		std::string path = "";
		VkDescriptorSet descriptor_set = VkDescriptorSet();
		//VulkanDevice* device;

		void destroy(VkDevice& device);
		VkResult CreateTexture(VulkanDevice& device);
		void updateDescriptor();
	};

	struct PrPixel {
		unsigned char r = 0, g = 0, b = 0, a = 1;
		unsigned char& operator[](int i) {
			assert(i > -1 && i < 4);
			return *(&r + i);
		}
		PrPixel() {};
	};

	struct PrImage {
		int width = 0, height = 0, channels = 0;
		std::vector<std::vector<PrPixel>> data = std::vector<std::vector<PrPixel>>();
		PrImage(int image_width, int image_height, int image_channels) : width(image_width), height(image_height), channels(image_channels) {
			data = std::vector(width, std::vector<PrPixel>(height));
		}
		PrImage(std::string txtr_file) {
			LoadPrImageFromTexture(txtr_file);
		}
		PrImage() {};
		void LoadPrImageFromTexture(std::string txtr_file) {
			stbi_uc* pixels = stbi_load(txtr_file.c_str(), &width, &height, &channels,0);
			data = std::vector(width, std::vector<PrPixel>(height));
			int i, j, k;
			for (k = 0; k < channels; ++k) {
				for (j = 0; j < height; ++j) {
					for (i = 0; i < width; ++i) {
						 int index = k + channels * i + channels * width * j;
						data[i][j][k] = pixels[index];
					}
				}
			}
			stbi_image_free(pixels);
		}
	};


}