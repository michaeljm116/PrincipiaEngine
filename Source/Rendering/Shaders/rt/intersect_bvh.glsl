#ifndef _INTERSECT_BVH_GLSL
#define _INTERSECT_BVH_GLSL
#include "structs.glsl"

bool bvhIntersect(inout Ray ray, in BVHNode node) {

	vec3 upperBounds = node.center + node.extents;
	vec3 lowerBounds = node.center - node.extents;

	vec3 invDir = 1 / ray.d;
	float tMin = FLT_MIN;
	float tMax = FLT_MAX;

	for (int i = 0; i < 3; ++i) {
		float t1 = (upperBounds[i] - ray.o[i]) * invDir[i];
		float t2 = (lowerBounds[i] - ray.o[i]) * invDir[i];

		tMin = max(tMin, min(t1, t2));
		tMax = min(tMax, max(t1, t2));
	}
	
	return (tMax > max(tMin, 0.0));
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