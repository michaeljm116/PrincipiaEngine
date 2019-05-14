#pragma once
/*Texture class Copyright (C) by Mike Murrell 2017
honestly,,, i dont even know if i still use this class
nvm turns out i still use it
nvm turns out i still use it
*/
//#include "RenderHelplers.h"
#include "vulkanbase.h"
struct Texture
{
public:
	Texture();
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

	//VulkanDevice* device;

	void destroy(VkDevice& device);
	VkResult CreateTexture(VulkanDevice& device);
	void updateDescriptor();
};

