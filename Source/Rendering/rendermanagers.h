#pragma once
/* RenderManagers Copyright (C) by Mike Murrell 2017
this is most likely legacy tbh, might need it later
if i ever decide to add textures to the game
*/
#ifndef  RENDERMANAGERS_H_
#define	 RENDERMANAGERS_H_

#define TEXTUREMANAGER RenderManager::get().getTextureManager()

#include "Texture.h"
#include "constantbuffer.hpp"
#include <glm/glm.hpp>
//#include "../pch.h"

/*
template<typename T>
inline TextureManager TEXTUREMANAGERX() {
	return RenderManager::get().getTextureManager();
}
}*/

namespace Principia {
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + filename);
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

}

#endif // ! RENDERMANAGERS_H
