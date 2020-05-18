/* Copyright (C) Mike Murrell 2017-2018 
ResouceManager class Global class that 
has a list of all the resources
*/

#pragma once
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

//#include <xxhash.hpp>
#include "../tinyxml2/tinyxml2.h"
#include "../Utility/componentIncludes.h"
#include <filesystem>
#include "../Rendering/shaderStructures.hpp"
#include "resourceStructs.h"
#include "helpers.h"
namespace Principia {
#define RESOURCEMANAGER Resources::get()

	//namespace fs = std::filesystem;

	class Resources
	{
	private:
		Resources();
		std::string materialsFileName;
		int numMeshFiles;
		rConfig					 config;
		std::vector<std::string> fileNames;
		std::vector<rModel>		 models;
		std::vector<rMaterial>	 materials;
		std::vector<rPoseList>	 poses;

	public:
		~Resources() {};
		static Resources& get() { static Resources instance; return instance; }
		Resources(Resources const&) = delete;
		void operator=(Resources const&) = delete;

		bool LoadConfig(std::string fileName);
		bool LoadDirectory(std::string directory);
		bool LoadAnimations(std::string directory);
		bool LoadResources(std::string fileName);
		bool LoadPModel(std::string fileName);
		bool LoadPose(std::string fileName, std::string prefabName);

		tinyxml2::XMLError  SaveMaterials();
		tinyxml2::XMLError LoadMaterials(const char* file);

		rConfig& getConfig() { return config; };

#pragma region Models
		inline rModel&		getModel(int i) { return models[i]; };
		inline rModel&		getModel(std::string n) {
			for (auto& m : models) {
				if (m.name == n)
					return m;
			}
			return models[0];
		}
		inline rModel&		getModelU(int i) {
			for (std::vector<rModel>::iterator itr = models.begin(); itr != models.end(); ++itr) {
				if (itr->uniqueID == i) {
					return *itr;
				}
			}
		}
		inline int			getModelIndex(std::string n) {
			for (size_t i = 0; i < models.size(); ++i) {
				if (models[i].name == n)
					return i;
			}
			return 0;
		}
		inline int			getModelIndex(int n) {
			for (size_t i = 0; i < models.size(); ++i) {
				if (models[i].uniqueID == n)
					return i;
			}
			return 0;
		}

		inline std::vector<rModel> getModels() { return models; };

#pragma endregion

		//const rPose& getPose(const std::string& prefabName, const std::string& poseName);
		const rPose& getPose(const int prefabName, const int poseName);

#pragma region materials
		inline rMaterial& getMaterial(int i) { return materials[i]; };
		inline rMaterial getMaterial(std::string n) {
			for (auto itr = materials.begin(); itr != materials.end(); ++itr) {
				if (itr->name == n)
					return *itr;
			}
			return rMaterial();
		}
		inline int		  getMaterialIndex(std::string n) {
			for (size_t i = 0; i < materials.size(); ++i) {
				if (materials[i].name == n)
					return i;
			}
			return 0;
		}
		inline rMaterial getMaterialU(int i) {
			for (std::vector<rMaterial>::const_iterator itr = materials.begin(); itr != materials.end(); ++itr) {
				if (itr->uniqueID == i)
					return *itr;
				return rMaterial();
			}
		}
		inline int getMaterialIndexU(int i) {
			for (size_t j = 0; j < materials.size(); ++j) {
				if (materials[j].uniqueID == i)
					return j;
			}
			return 0;
		}
		inline std::vector<rMaterial> getMaterials() { return materials; };
		inline void addMaterial(std::string s, glm::vec3 diff, float r, float rg, float tr, float ri, int ti) {
			rMaterial mat = rMaterial(s, diff, r, rg, tr, ri, ti);
			materials.push_back(mat);
		}
#pragma endregion 


	private:

		tinyxml2::XMLError loadScript(const char* file);
		std::string nameMesh(std::string fn);

	};
}

#endif // !RESOURCEMANAGER_H