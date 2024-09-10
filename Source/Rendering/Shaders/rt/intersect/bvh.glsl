#ifndef _INTERSECT_BVH_GLSL
#define _INTERSECT_BVH_GLSL
#include "../structs.glsl"

vec3 bs(in BVHNode node, int index) {
	if(index == 0) return node.lower; else return node.upper;
	//return vec3(0, 0, 0);
}

ivec3 DirectionIsNegative(in vec3 invDir) {
	ivec3 ret;
	//ret[0] = (int)invDir[0];
	//ret[1] = (int)invDir[1];
	//ret[2] = (int)invDir[2];
	for (int i = 0; i < 3; ++i) {
		//ret[i] = 1 ^ (ret[i] >> 31);
		if (invDir[i] < 0)
			ret[i] = 0;
		else
			ret[i] = 1;
	}
	return ret;

}

bool bvhIntersect(inout Ray ray, vec3 invDir, ivec3 dirIsNeg, in BVHNode bounds) {
	//const Bounds3f &bounds = *this;
	// Check for ray intersection against $x$ and $y$ slabs
	float tMin = (bs(bounds, dirIsNeg[0]).x - ray.o.x) * invDir.x;
	float tMax = (bs(bounds, 1 - dirIsNeg[0]).x - ray.o.x) * invDir.x;
	float tyMin = (bs(bounds, dirIsNeg[1]).y - ray.o.y) * invDir.y;
	float tyMax = (bs(bounds, 1 - dirIsNeg[1]).y - ray.o.y) * invDir.y;

	// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
	//tMax *= 1 + 2 * gamma(3);
	//tyMax *= 1 + 2 * gamma(3);
	if (tMin > tyMax || tyMin > tMax) return false;
	if (tyMin > tMin) tMin = tyMin;
	if (tyMax < tMax) tMax = tyMax;

	// Check for ray intersection against $z$ slab
	float tzMin = (bs(bounds, dirIsNeg[2]).z - ray.o.z) * invDir.z;
	float tzMax = (bs(bounds, 1 - dirIsNeg[2]).z - ray.o.z) * invDir.z;

	// Update _tzMax_ to ensure robust bounds intersection
	//tzMax *= 1 + 2 * gamma(3);
	if (tMin > tzMax || tzMin > tMax) return false;
	if (tzMin > tMin) tMin = tzMin;
	if (tzMax < tMax) tMax = tzMax;
	//return (tMin < ray.tbvh) && (tMax > 0);
	//if ((tMin < ray.tbvh) && (tMax > 0)) {
	//	ray.tbvh = tMin;
	//	return true;
	//}
	//else return false;
	return (tMax > 0);
}

bool mbvhIntersect(Ray ray, vec3 invDir, in BVHNode node) {

	//vec3 invDir = 1 / ray.d;
	float tMin = FLT_MIN;
	float tMax = FLT_MAX;

	for (int i = 0; i < 3; ++i) {
		//float t1 = (node.upper[i] - ray.o[i]) * invDir[i];
		//float t2 = (node.lower[i] - ray.o[i]) * invDir[i];
		float t1 = -ray.o[i] * invDir[i] + (invDir[i] * node.upper[i]);
		float t2 = -ray.o[i] * invDir[i] + (invDir[i] * node.lower[i]);

		tMin = max(tMin, min(t1, t2));
		tMax = min(tMax, max(t1, t2));
	}

	return (tMax > max(tMin, 0.0));
}

bool bvhBoundsIntersect(Ray ray, vec3 invDir, in BVHNode node){
	float tMin = FLT_MIN;
	float norm = 0;
	//float tMax = FLT_MAX;
	for (int i = 0; i < 3; ++i) {
		//float t1 = (node.upper[i] - ray.o[i]) * invDir[i];
		//float t2 = (node.lower[i] - ray.o[i]) * invDir[i];
		float t1 = -ray.o[i] * invDir[i] + (invDir[i] * node.upper[i]);
		float t2 = -ray.o[i] * invDir[i] + (invDir[i] * node.lower[i]);

		//tMin = max(tMin, min(t1, t2));
		//tMax = min(tMax, max(t1, t2));
		float temp = min(t1, t2);
		if(temp > tMin){tMin = temp; norm = i;}
	}
	vec3 pos = ray.d * tMin + ray.o;
	vec3 cen = 0.5f * (node.upper + node.lower);
	vec3 ext = abs(node.upper - cen);
	for (int i = 0; i < 3; ++i){
		if(i != norm){
			float p = abs(pos[i] - cen[i]);
			if((p > ext[i] - 0.01f) && (p < ext[i])){
				return true;
			}
		}
	}

	return false;
}


#endif

/*
assume you always colldie with 1
if you collide with child2  push child 3 into a queue
if you collide with child 26 push child 26 into a queue

for current queue size pop th equeue and add on these
if you collide with child 4 push child 5 into a queue
if you collide with child 14 push child 14 into a queue
if you collide with child 27 push child 28 into a queue
if you collide with child 43 push child 43 into a queue

problem is max queue size stuff like what happens when you reach 64


So theres mutli way sto do this should i....
If you intersect me
	add both my cihldren to the queue

orrrrrr

If you intersect my children
	add the child you intersect into the queue


So how do you keeep track fo a queeueueueueue

so to pop something off a queue you....
move the start of the pointer forward

to push something into a queue you
move the end of the pointer forward
and you put something at the end of that pointer

So whith your loop the problem is you'll keep internally expanding the loop
if size is 1 then keep the size at 1 but then etc nnawmsayin

so like have an external qStart and end
everytime you pop you remoe and then go back
and then at the end of hte while loop you update the queue size





*/
