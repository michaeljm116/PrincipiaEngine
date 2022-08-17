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
		int width, height, channels;
		std::vector<std::vector<PrPixel>> data;
		PrImage(int image_width, int image_height, int image_channels) : width(image_width), height(image_height), channels(image_channels) {
			data = std::vector(width, std::vector<PrPixel>(height));
		}
		PrImage(std::string txtr_file) {
			LoadPrImageFromTexture(txtr_file);
		}
		void LoadPrImageFromTexture(std::string txtr_file) {
			stbi_uc* pixels = stbi_load(txtr_file.c_str(), &width, &height, &channels,0);
			data = std::vector(width, std::vector<PrPixel>(height));
			unsigned int i, j, k;
			for (k = 0; k < channels; ++k) {
				for (j = 0; j < height; ++j) {
					for (i = 0; i < width; ++i) {
						unsigned int index = k + channels * i + channels * width * j;
						data[i][j][k] = pixels[index];
					}
				}
			}
			stbi_image_free(pixels);
		}
	};


}