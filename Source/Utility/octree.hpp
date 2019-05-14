
#pragma once
#ifndef OCTREE_H
#define OCTREE_H
#endif // !OCTREE_H

#include "../pch.h"
#include "../Game/primitive.hpp"

//static bool g_treeReady = false;		//the tree has a few objects which need to be inserted before
//static bool g_treeBuilt = false;		//there is no pre-existing tree yet.
template <typename T>
class Octree {
public:
	Octree() {};
	Octree(AABB region, std::vector<T*> ents) {
		m_region = region;
		m_objects = ents;
		for (int i = 0; i < m_objects.size(); ++i) {
			m_objects[i]->nodeCenter = &m_region.center;
		}
	};
	~Octree() {
		for (int i = 0; i < 8; ++i) {
			if (m_ChildNode[i] != NULL)
				delete(m_ChildNode[i]);
		}
	};

	AABB m_region;						//Bounding Box of the octree
	std::vector<T*> m_objects;			 //List of entities it holds


	Octree* m_ChildNode[8];				//Possible children for the node

	char m_activeNodes = 0;				//Indicates which child nodes are being used
	const float MIN_SIZE = 0.05f;		//minimum size for enclsing region is a 1x1x1 cube.
	int m_maxLifeSpan = 8;
	int m_curLife = -1;					//This is a countdowntime showing how much life we have left to live???

	Octree* parent;

	//static bool m_treeReady;			//the tree has a few objects which need to be inserted before
	//static bool m_treeBuilt;			//there is no pre-existing tree yet.

	void BuildTree() {
		if (m_objects.size() <= 1)
			return;

		//make sure noting goes less than min size, ends the recursion
		for (int i = 0; i < 3; ++i) {
			if (m_region.extents[i] < MIN_SIZE)
				return;
		}

		glm::vec3 minRegion = m_region.center - m_region.extents;
		glm::vec3 maxRegion = m_region.center + m_region.extents;
		glm::vec3 maxCenter = m_region.center + (m_region.extents * 0.5f);
		glm::vec3 minCenter = m_region.center - (m_region.extents * 0.5f);
		glm::vec3 halfExtents = m_region.extents * 0.5f;

		// create subdivided regions for each octant
		AABB octant[8] = {
			AABB(maxCenter, halfExtents),// maxRegion),
			AABB(glm::vec3(maxCenter.x, maxCenter.y, minCenter.z), halfExtents),
			AABB(glm::vec3(maxCenter.x, minCenter.y, maxCenter.z), halfExtents),
			AABB(glm::vec3(maxCenter.x, minCenter.y, minCenter.z), halfExtents),
			AABB(minCenter, halfExtents),
			AABB(glm::vec3(minCenter.x, minCenter.y, maxCenter.z), halfExtents),
			AABB(glm::vec3(minCenter.x, maxCenter.y, minCenter.z), halfExtents),
			AABB(glm::vec3(minCenter.x, maxCenter.y, maxCenter.z), halfExtents)
		};

		//see stuffz0s
		std::vector<T*> octList[8];
		std::vector<T*> keptObjects;
		for (int i = 0; i < m_objects.size(); ++i) {
			bool kept = true;
			for (int b = 0; b < 8; b++) {
				if (octant[b].Contains(m_objects[i])) {
					octList[b].push_back(m_objects[i]);
					kept = false;
				}
			}
			if (kept)
				keptObjects.push_back(m_objects[i]);
		}

		//m_objects = keptObjects; //CHANGE THIS LATER SO YOU CAN MEMORY SWAP STUFF
		// build the nodes
		for (int b = 0; b < 8; b++) {
			if (octList->size() > 0) {
				m_ChildNode[b] = CreateNode(octant[b], octList[b]);
				m_activeNodes = 1 << b;
				m_ChildNode[b]->BuildTree();
			}
		}
		m_objects = keptObjects;

		//g_treeBuilt = true;
		//g_treeReady = true;
	}

	void Insert(T* ent) {
		Insert(ent, this);
	}
	void Insert(T* ent, Octree* par) {
		//Make sure it fits inside the parent, if not then go up
		if (!par->m_region.Contains(ent) && par->parent != nullptr) {
			Insert(ent, par->parent);
		}

		bool inChild = false;
		//See if it fits inside any of the children
		for (int i = 0; i < 8; ++i) {
			if (par->m_ChildNode[i]->m_region.Contains(ent)) {

				inChild = true;
			}
		}

		//if not then make new child
		if (!inChild) {
			glm::vec3 minRegion = par->m_region.center - par->m_region.extents;
			glm::vec3 maxRegion = par->m_region.center + par->m_region.extents;
			glm::vec3 maxCenter = par->m_region.center + (par->m_region.extents * 0.5f);
			glm::vec3 minCenter = par->m_region.center - (par->m_region.extents * 0.5f);
			glm::vec3 halfExtents = par->m_region.extents * 0.5f;

			// create subdivided regions for each octant
			AABB octant[8] = {
				AABB(maxCenter, halfExtents),// maxRegion),
				AABB(glm::vec3(maxCenter.x, maxCenter.y, minCenter.z), halfExtents),
				AABB(glm::vec3(maxCenter.x, minCenter.y, maxCenter.z), halfExtents),
				AABB(glm::vec3(maxCenter.x, minCenter.y, minCenter.z), halfExtents),
				AABB(minCenter, halfExtents),
				AABB(glm::vec3(minCenter.x, minCenter.y, maxCenter.z), halfExtents),
				AABB(glm::vec3(minCenter.x, maxCenter.y, minCenter.z), halfExtents),
				AABB(glm::vec3(minCenter.x, maxCenter.y, maxCenter.z), halfExtents)
			};

			//Find which octant its in
			int ind = 0;
			for (ind; ind < 8; ++ind) {
				if (octant[ind].Contains(ent)) {
					break;
				}
			}

			//Create node for that octant;
			par->m_ChildNode[ind] = CreateNode(octant[ind], ent, par);

		}
	}
	void Update(T* ent) {
		Update(ent, this);
	}
	void Update(T* ent, Octree* par) {
		// Find which child object to update
	}

	Octree* CreateNode(AABB region, std::vector<T*> ents, Octree* par) {
		Octree<T>* octo = new Octree(region, ents);
		octo->parent = par;
		return octo;
	}
	Octree* CreateNode(AABB region, std::vector<T*> ents) {
		return CreateNode(region, ents, this);
	}
	Octree* CreateNode(AABB region, T* ent, Octree* par) {
		std::vector<T*> ents;
		ents.push_back(ent);
		return CreateNode(region, ents, par);
	}
	Octree* CreateNode(AABB region, T* ent) {
		return CreateNode(region, ent, this);
	}

	//Cencter Check is based off the idea that the octree is organized as :
	//XYZ XYz XyZ Xyz______xyz xyZ xYz xYZ
	int CenterCheck(glm::vec3 a, glm::vec3 b) {
		if (a.x > b.x) {
			if (a.y > b.y) {
				if (a.z > b.z)
					return 0;
				else
					return 1;
			}
			else {
				if (a.z > b.z)
					return 2;
				else
					return 3;
			}
		}
		else {
			if (a.y < b.y) {
				if (a.z < b.z)
					return 4;
				else
					return 5;
			}
			else {
				if (a.z < b.z)
					return 6;
				else
					return 7;
			}

		}
		return -1; //They must all be equal?
	}
};