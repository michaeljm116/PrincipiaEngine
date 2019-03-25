#pragma once
//#include "../Game/transform.hpp"
#include "../Game/primitive.hpp"
#include <vector>

//This class is a helper so that you can properly transform an object to fit a particular scale
//Should probably be renamed to something like Unit or unit scaler because its transforming into a particular uniform unit of measurement
class Scaler {
public:
	Scaler() { world = glm::mat4(1); };
	void Scale(float size, std::vector<Mesh>& meshes, int mStart, int mEnd, std::vector<TriangleVert>& verts, int vStart, int vEnd) {
		world = glm::mat4(1);
		//Find max extent
		float maxE = FLT_MIN;
		for (int i = mStart; i < mEnd; i++) {
			for (int j = 0; j < 3; ++j) {
				if (maxE < meshes[i].extents[j])
					maxE = meshes[i].extents[j];
			}
		}

		//compare it to size;
		float ratio = size / maxE;

		////Transfomr the AABBs to fit the size
		//for (int i = mStart; i < mEnd; ++i) {
		//	meshes[i].extents *= ratio;
		//}

		//Transform the bounds to fit the size
		world = glm::scale(world, glm::vec3(ratio));
		for (int i = mStart; i < mEnd; ++i) {
			meshes[i].center = glm::vec3(world * glm::vec4(meshes[i].center, 1.f));
			meshes[i].extents = glm::vec3(world * glm::vec4(meshes[i].extents, 1.f));
		}

		//Transform the verts to fit the size
		for (int i = vStart; i < vEnd; ++i) {
			verts[i].pos = glm::vec3(world * glm::vec4(verts[i].pos, 1.f));
		}

	}
	void Scale(float size, AABB& bounds, std::vector<glm::vec3>& verts, int start, int end) {
		
		//Find max extent;
		float maxE = FLT_MIN;
		for (int i = 0; i < 3; ++i) {
			if (maxE > bounds.extents[i])
				maxE = bounds.extents[i];
		}
		
		//Compare it to size
		float ratio = size / maxE;

		//Transform the AABB to fit the size
		for (int i = 0; i < 3; ++i)
			bounds.extents[i] *= ratio;

		//Transform the bounds to fit the size
		glm::vec3 size3 = glm::vec3(size);
		world = glm::scale(world, size3);
		bounds.center = glm::vec3(world * glm::vec4(bounds.center, 1.f));
		bounds.extents = glm::vec3(world * glm::vec4(bounds.extents, 1.f));

		//Transform the verets to fit the size
		for (int i = start; i < end; ++i) {
			verts[i] = glm::vec3(world * glm::vec4(verts[i], 1.f));
		}
	};

	void Scale(float size, AABB& bounds, std::vector<TriangleVert>& verts, int start, int end) {

		//Find max extent;
		float maxE = FLT_MIN;
		for (int i = 0; i < 3; ++i) {
			if (maxE > bounds.extents[i])
				maxE = bounds.extents[i];
		}

		//Compare it to size
		float ratio = size / maxE;

		//Transform the AABB to fit the size
		for (int i = 0; i < 3; ++i)
			bounds.extents[i] *= ratio;

		//Transform the bounds to fit the size
		glm::vec3 size3 = glm::vec3(size);
		world = glm::scale(world, size3);
		bounds.center = glm::vec3(world * glm::vec4(bounds.center, 1.f));
		bounds.extents = glm::vec3(world * glm::vec4(bounds.extents, 1.f));

		//Transform the verets to fit the size
		for (int i = start; i < end; ++i) {
			verts[i].pos = glm::vec3(world * glm::vec4(verts[i].pos, 1.f));
		}
	};

	void Scale(float size, AABB& bounds, std::vector<glm::vec3>& verts) {
		Scale(size, bounds, verts, 0, verts.size());
	}



private:
	glm::mat4 world;
};

/*
problem: This scales every individual 
*/