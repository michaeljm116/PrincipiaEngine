/* Copyright (C) Mike Murrell 2019 Resource structs
just a bunch of structures used for
the resource manager
*/
//#include <glm/glm.hpp>
//#include "../pch.h"

//Structure for the config file, will expand moar l8r

#pragma once

#include <vector>
#include <string>
#include <array>
#include <iostream>
#include <glm/glm.hpp>
#include "bvhComponent.hpp"

namespace Principia {

    struct ssMaterial;
    struct sqt;

    const float BONE_EPSILON = 0.05f;

    struct rController {
        std::array<int, 16> buttons{};
        std::array<float, 6> axis{};
    };

    struct rConfig {
        int numControllersConfigs = 0;
        std::vector<rController> controllerConfigs;
    };

    struct rJointData {
        std::array<int, 4> id{};
        std::array<float, 4> weights{};

        void average() noexcept {
            float total = 0;
            for (int i = 0; i < 4; ++i) {
                if (weights[i] < BONE_EPSILON)
                    weights[i] = 0;
                total += weights[i];
            }
            if (total == 0) {
                return;
            }
            for (int i = 0; i < 4; ++i) {
                weights[i] /= total;
            }
        }
    };

    struct rMaterial {
        glm::vec3 diffuse{};
        float reflective = 0;
        float roughness = 0;
        float transparency = 0;
        float refractiveIndex = 0;
        int textureID = -1;
        int uniqueID = 0;
		std::string Texture = "";
		std::string name = "";
        ssMaterial* renderedMat = nullptr;

        rMaterial() = default;

        rMaterial(std::string n, glm::vec3 d, float rfl, float rgh,
            float trns, float rfr, int ti) noexcept
            : name(n), diffuse(d), reflective(rfl), refractiveIndex(rfr),
            roughness(rgh), transparency(trns), textureID(ti) {

            uniqueID = name[0];
            for (size_t i = 1; i < name.size(); ++i) {
                uniqueID *= name[i] + name[i - 1];
            }
        }
    };

    struct rVertex {
        glm::vec3 pos{};
        glm::vec3 norm{};
        glm::vec2 uv{};

        rVertex() = default;

        rVertex(const glm::vec3& p, const glm::vec3& n) noexcept : pos(p), norm(n) {}
    };

    struct rMesh {
        std::vector<rVertex> verts;
        std::vector<glm::ivec4> faces;
        std::vector<ssBVHNode> bvh;
        std::vector<rJointData> bones;
        glm::vec3 center{};
        glm::vec3 extents{};
        std::string name;
        rMaterial mat;
        int matId = 0;
        int meshID = 0;
    };

    struct rShape {
        std::string name;
        int type = 0;
        glm::vec3 center{};
        glm::vec3 extents{};
    };

    struct rModel {
        std::string name;
        std::vector<rMesh> meshes;
        std::vector<rShape> shapes;
        glm::vec3 center{};
        glm::vec3 extents{};
        int uniqueID = 0;
        int skeletonID = 0;
        bool skinned = false;
        bool triangular = false;
    };

    struct rPose {
        std::string name;
        std::vector<std::pair<int, sqt>> pose;
        int hashVal = 0;

        ~rPose() = default;
    };

    struct rPoseList {
        std::string name;
        std::vector<rPose> poses;
        int hashVal = 0;
    };
}
