#include "../pch.h"
#include "Texture.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

namespace Principia {
	Texture::Texture()
	{
		descriptor = {};
	}


	Texture::Texture(std::string p, VulkanDevice& device) : path(p)
	{
		CreateTexture(device);
	}

	Texture::~Texture()
	{
		//destroy(device->logicalDevice);
	}

	void Texture::destroy(VkDevice& device)
	{
		vkDestroySampler(device, sampler, nullptr);
		vkDestroyImageView(device, view, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkFreeMemory(device, memory, nullptr);
	}

	VkResult Texture::CreateTexture(VulkanDevice & device)
	{
		//////////////CREATE TEXTURE IMAGE/////////////////
		//this->device = &device;
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		height = static_cast<uint32_t>(texHeight);
		width = static_cast<uint32_t>(texWidth);

		if (!pixels) throw std::runtime_error("failed to load texture image!");

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device.logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device.logicalDevice, stagingBufferMemory);
		stbi_image_free(pixels);

		device.createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);

		device.transitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		device.copyBufferToImage(stagingBuffer, image, width, height);
		device.transitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device.logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(device.logicalDevice, stagingBufferMemory, nullptr);
		/////////////////////////////////////////////////////
		////////////CREATE TEXUTRE IMAGE VIEW////////////////
		view = device.createImageView(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
		/////////////////////////////////////////////////////
		/////////////CREATE TEXTURE SAMPLER//////////////////
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		VK_CHECKRESULT(vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr, &sampler), "create texture sampler!");


		imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		updateDescriptor();

		return VkResult();
	}

	void Texture::updateDescriptor()
	{
		descriptor.sampler = sampler;
		descriptor.imageView = view;
		descriptor.imageLayout = imageLayout;
	}
}