#pragma once
/* RenderManagers Copyright (C) by Mike Murrell 2017
this is most likely legacy tbh, might need it later
if i ever decide to add textures to the game
*/
#ifndef  RENDERMANAGERS_H_
#define	 RENDERMANAGERS_H_

#define TEXTUREMANAGER RenderManager::get().getTextureManager()

#include "Texture.h"
#include "model.h"
#include "light.hpp"
#include "constantbuffer.hpp"
#include "../pch.h"

/*
template<typename T>
inline TextureManager TEXTUREMANAGERX() {
	return RenderManager::get().getTextureManager();
}
}*/

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

struct PerFrameCB
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 model;

};
struct PerObjectCB
{
	glm::mat4 world;
	glm::mat4 texTransform;
};


class RenderManager {
	struct ShaderManager {
	};

	struct TextureManager {
		std::vector<Texture> diffuseTextureList;
		std::vector<Texture> specularTextureList;
		std::vector<Texture> normalTextureList;

		std::vector<std::string> diffuseTextureNameArray;
		std::vector<std::string> specularTextureNameArray;
		std::vector<std::string> normalTextureNameArray;

		VkResult LoadTextures(VulkanDevice& device, std::string texDir) {
			size_t i;
			//For Diffuse Textures
			for (i = 0; i < diffuseTextureNameArray.size(); i++)
			{
				Texture diffTex;
				diffTex.path = texDir + diffuseTextureNameArray[i];
				diffTex.CreateTexture(device);
				diffuseTextureList.push_back(diffTex);
			}

			//For Specular Map Textures
			for (i = 0; i < specularTextureNameArray.size(); i++)
			{
				Texture specTex;
				specTex.path = texDir + specularTextureNameArray[i];
				specTex.CreateTexture(device);
				specularTextureList.push_back(specTex);
			}

			//For Normal Map Textures
			for (i = 0; i < normalTextureNameArray.size(); i++)
			{
				Texture normTex;
				normTex.path = texDir + normalTextureNameArray[i];
				normTex.CreateTexture(device);
				normalTextureList.push_back(normTex);
			}

			return VK_SUCCESS;
		}
		void DestroyTextures(VkDevice& device) {
			for each (Texture txt in diffuseTextureList)
			{
				txt.destroy(device);
			}
			for each (Texture txt in specularTextureList)
			{
				txt.destroy(device);
			}
			for each (Texture txt in normalTextureList)
			{
				txt.destroy(device);
			}
		}
	};

	TextureManager textureMan;

public:
	RenderManager() {}
	~RenderManager();
	static RenderManager& get() {static RenderManager instance; return instance;}
	RenderManager(RenderManager const&) = delete;
	void operator=(RenderManager const&) = delete;

	TextureManager*  getTextureManager() { return &textureMan; }

	VulkanDevice* vkDevice;
};

#endif // ! RENDERMANAGERS_H
