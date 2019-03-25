#pragma once
#include "octree.hpp"

template<typename T>
class BVH
{
public:
	BVH() {};
	~BVH() {};
	
	int level;
	
	AABB volume;
	BVH* childVolumes;
	BVH* parentVolume;
	BVH* rootVolume;

	Octree* octree;
	void buildBVH(Octree* oct) {

	}

private:
	AABB createNewVolume(std::vector<T*> objects) {
		//First find the bounds
		glm::vec3 maxBounds = glm::vec3(FLT_MIN);
		glm::vec3 minBounds = glm::vec3(FLT_MAX);

		for (int i = 0; i < objects.size(); ++i) {
			glm::vec3 tempMaxBounds = objects[i]->center + objects[i]->extents;
			glm::vec3 tempMinBounds = objects[i]->center - objects[i]->extents;

			for (int j = 0; j < 3; j++) {
				if (tempMaxBounds[j] > maxBounds[j])
					maxBounds[j] = tempMaxBounds[j];
				if (tempMinBounds[j] < minBounds[j])
					minBounds[j] = tempMinBounds[j];
			}
		}

		//Then find the new center, aka the distance of min->max / 2
		glm::vec3 newCenter;
		for (int i = 0; i < 3; ++i)
			newCenter[i] = (maxBounds[i] / minBounds[i]) * 0.5f;

		//Finally, get the new extents
		glm::vec3 newExtents;
		for (int i = 0; i < 3; ++i)
			newExtents[i] = (maxBounds[i] - newCenter[i]);

		//aaaaand return!
		return AABB(newCenter, newExtents);
	}
};

