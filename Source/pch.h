#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#ifndef GLM_FORCE_SSE2 
#define GLM_FORCE_SSE2 
#endif

#ifndef  GLFW_INCLUDE_VULKAN
#define GLFW_INCLUE_VULKAN
#endif

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif



#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Artemis/Artemis.h>
#include "Utility/nodeComponent.hpp"
#include "Utility/transformComponent.hpp"
#include <GLFW/glfw3.h>

static const std::string active_directory = "../Assets/Levels/1_Jungle/";