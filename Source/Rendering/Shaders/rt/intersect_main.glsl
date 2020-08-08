#ifndef _INTERSECT_MAIN_GLSL
#define _INTERSECT_MAIN_GLSL

#include "intersect_plane.glsl"
#include "intersect_box.glsl"
#include "intersect_sphere.glsl"
#include "intersect_mesh.glsl"
#include "intersect_cylinder.glsl"
#include "intersect_disk.glsl"
#include "intersect_bvh.glsl"

#define TRIINTERSECT true
#define SHADOW 0.2
//#define DEBUGLINES 

const uint TYPE_SPHERE = 0x1u;
const uint TYPE_BOX = 0x2u;
const uint TYPE_PLANE = 0x4u;
const uint TYPE_MESH = 0x8u;
const uint TYPE_CYLINDER = 0x10u;
const uint TYPE_DISK = 0x20u;
const uint TYPE_JOINT = 0x40u;
const uint TYPE_BVHNODE = 0x80u;
const int BIT_000_MAX = 268435455;

iSectID intersectMBVH(inout Ray ray, inout vec3 norm) {
	iSectID id = iSectID(0, -1, -1, -1);
	int stack[16];
	int sp = 0; //stack pointer
	stack[0] = 0;	
	vec3 invDir = 1 / ray.d;

	while (sp > -1) {
		int offset = stack[sp];
		BVHNode node = bvhNodes[offset];
		sp--;
		//int numChildren = node.numChildren & BIT_000_MAX;
		//int axis = node.numChildren >> 29;
		#ifdef DEBUGLINES
		if(bvhBoundsIntersect(ray, invDir, bvhNodes[offset])){
			return iSectID(0, 0, 0, offset);
		}
		#endif
		//if its a leaf do the regular intersection
		if (node.numChildren > 0) {
			for (int i = node.offset; i < node.offset + node.numChildren; ++i) {
				if (primitives[i].id > 10) {
					mat4 invWorld = inverse(primitives[i].world);
					Ray r;
					r.d = (invWorld*vec4(ray.d, 0.0)).xyz;// / primitives[i].extents;
					r.o = (invWorld*vec4(ray.o, 1.0)).xyz;// / primitives[i].extents;
					flool tMesh = boundsIntersect(r);// , vec3(1, 1, 1));// primitives[i].extents);
					if (tMesh.b && (tMesh.t > EPSILON) && (tMesh.t < ray.t)) { //hits the boundingbox, doesnt necessarily mean tri hit
						int startIndex = primitives[i].startIndex;
						int endIndex = primitives[i].endIndex;
						for (int f = startIndex; f < endIndex; f++) {
							vec4 tQuad = quadIntersect(r, faces[f]);
							if ((tQuad.x > 0) && (tQuad.x > EPSILON) && (tQuad.x < ray.t)) {
								id = iSectID(TYPE_MESH, f, i, offset);
								ray.t = tQuad.x;
								norm.x = tQuad.y;
								norm.y = tQuad.z;
							}
						}
						//sectID temp = intersectPrimBVH(ray, norm, r, primitives[i].startIndex, primitives[i].endIndex);
						//if (temp.id != -1) {
						//	id = temp;
						//	id.pId = i;
						//}
					}
				}//id > 0

				else if (primitives[i].id == -1) { //SPHERE INTERSECT
					float tSphere = sphereIntersect(ray, primitives[i]);
					if ((tSphere > EPSILON) && (tSphere < ray.t))
					{
						id = iSectID(TYPE_SPHERE, i, -1, offset);
						ray.t = tSphere;
					}
				}
				else if (primitives[i].id == -2) { //BOX INTERSECT
					vec4 tBox = boxIntersect(ray, primitives[i]);
					if (tBox.x > 0) {
						if ((tBox.x > EPSILON) && (tBox.x < ray.t)) {
							id = iSectID(TYPE_BOX, i, -1, offset);
							ray.t = tBox.x;
							norm = tBox.yzw;
						}
					}
				}
				else if (primitives[i].id == -3) { //CYLINDER INTERSECT
					vec4 tCylinder = cylinderIntersect(ray, primitives[i]);
					if ((tCylinder.x > EPSILON) && (tCylinder.x < ray.t)) {
						id = iSectID(TYPE_CYLINDER, i, -1, offset);
						ray.t = tCylinder.x;
						norm = tCylinder.yzw;
					}
				}
				else if (primitives[i].id == -4) { //PLANE INTERSECT
					float tplane = planeIntersect(ray, primitives[i]);
					if ((tplane > EPSILON) && (tplane < ray.t))
					{
						id = iSectID(TYPE_PLANE, i, -1, offset);
						ray.t = tplane;
					}
				}
				else if (primitives[i].id == -5) { //DISK INTERSECT
					float tDisk = diskIntersect(ray, primitives[i]);
					if ((tDisk > EPSILON) && (tDisk < ray.t)) {
						id = iSectID(TYPE_DISK, i, -1, offset);
						ray.t = tDisk;
					}
				}
			}
		}
		//It's a node
		else { 
			offset = offset + 1;
			if (mbvhIntersect(ray, invDir, bvhNodes[offset])) {
				sp++;
				stack[sp] = offset;
			}
			if (node.offset < bvhNodes.length()) {
				if (mbvhIntersect(ray, invDir, bvhNodes[node.offset])) {
					sp++;
					stack[sp] = node.offset;
				}
			}
		}
	}
	


	return id;
}


//rayO = pos, rayD = light, primitiveId = duh, t = t = length(lights[0].pos - pos);
float calcShadowBVH(inout Ray ray){
	int stack[16];
	int sp = 0; //stack pointer
	stack[0] = 0;
	vec3 invDir = 1 / ray.d;

	while (sp > -1) {
		int offset = stack[sp];
		BVHNode node = bvhNodes[offset];
		sp--;
		//int numChildren = node.numChildren & BIT_000_MAX;
		//int axis = node.numChildren >> 29;

		//if its a leaf do the regular intersection
		if (node.numChildren > 0) {
			for (int i = node.offset; i < node.offset + node.numChildren; ++i) {
				if (primitives[i].id > 10) {
					mat4 invWorld = inverse(primitives[i].world);
					Ray r;
					r.d = (invWorld*vec4(ray.d, 0.0)).xyz;// / primitives[i].extents;
					r.o = (invWorld*vec4(ray.o, 1.0)).xyz;// / primitives[i].extents;
					flool tMesh = boundsIntersect(r);// , vec3(1, 1, 1));// primitives[i].extents);
					if (tMesh.b && (tMesh.t > EPSILON) && (tMesh.t < ray.t)) { //hits the boundingbox, doesnt necessarily mean tri hit
						//Mesh m = meshes[primitives[i].id];
						//id.pId = i;
						//rdd /= primitives[i].extents;
						//roo /= primitives[i].extents;
						int startIndex = primitives[i].startIndex;
						int endIndex = primitives[i].endIndex;
						for (int f = startIndex; f < endIndex; f++) {
							vec4 tQuad = quadIntersect(r, faces[f]);
							if ((tQuad.x > 0) && (tQuad.x > EPSILON) && (tQuad.x < ray.t)) {
								return SHADOW;
								ray.t = tQuad.x;
							}
						}
					}
				}//id > 0

				else if (primitives[i].id == -1) { //SPHERE INTERSECT
					float tSphere = sphereIntersect(ray, primitives[i]);
					if ((tSphere > EPSILON) && (tSphere < ray.t))
					{
						return SHADOW;
						ray.t = tSphere;
					}
				}
				else if (primitives[i].id == -2) { //BOX INTERSECT
					vec4 tBox = boxIntersect(ray, primitives[i]);
					if (tBox.x > 0) {
						if ((tBox.x > EPSILON) && (tBox.x < ray.t)) {
							return SHADOW;
							ray.t = tBox.x;
						}
					}
				}
				else if (primitives[i].id == -3) { //CYLINDER INTERSECT
					vec4 tCylinder = cylinderIntersect(ray, primitives[i]);
					if ((tCylinder.x > EPSILON) && (tCylinder.x < ray.t)) {
						return SHADOW;
						ray.t = tCylinder.x;
					}
				}
				else if (primitives[i].id == -4) { //PLANE INTERSECT
					float tplane = planeIntersect(ray, primitives[i]);
					if ((tplane > EPSILON) && (tplane < ray.t))
					{
						return SHADOW;
						ray.t = tplane;
					}
				}
				else if (primitives[i].id == -5) { //DISK INTERSECT
					float tDisk = diskIntersect(ray, primitives[i]);
					if ((tDisk > EPSILON) && (tDisk < ray.t)) {
						return SHADOW;
						ray.t = tDisk;
					}
				}
			}
		}
		//It's a node
		else {
			offset = offset + 1;
			if (mbvhIntersect(ray, invDir, bvhNodes[offset])) {
				sp++;
				stack[sp] = offset;
			}
			if (node.offset < bvhNodes.length()) {
				if (mbvhIntersect(ray, invDir, bvhNodes[node.offset])) {
					sp++;
					stack[sp] = node.offset;
				}
			}
		}
	}

	return 1.f;
}

float calcShadow(inout Ray r, in sectID primitiveId)
{
	/*
	for (int i = 0; i < primitives.length(); ++i) {
		if (primitives[i].id > -1) {/////-----MESH-----|||||
			mat4 invWorld = inverse(primitives[i].world);
			vec3 rdd = (invWorld*vec4(rayD, 0.0)).xyz;// / primitives[i].extents;
			vec3 roo = (invWorld*vec4(rayO, 1.0)).xyz;// / primitives[i].extents;

			flool tMesh = boundsIntersect(roo, rdd);// , vec3(1, 1, 1));// primitives[i].extents);
			if (tMesh.b && (tMesh.t > EPSILON) && (tMesh.t < t)) {
				int startIndex = primitives[i].startIndex;
				int endIndex = primitives[i].endIndex;
				for (int j = startIndex; j < endIndex; j++) {
					vec4 tQuad = quadIntersect(roo, rdd, faces[j]);
					if (tQuad.x > 0) {
						if ((tQuad.x > EPSILON) && (tQuad.x < t)) {
							t = tQuad.x;
							return SHADOW;
						}
					}
				}
			}
		}
		else if (primitives[i].id == --1) { /////-----SPHERE-----|||||
			float tSphere = sphereIntersect(rayO, rayD, primitives[i]);
			if ((tSphere > EPSILON) && (tSphere < t)) {
				t = tSphere;
				return SHADOW;
			}
		}
		else if (primitives[i].id == --2) { /////-----BOX-----|||||
			float tBox = boxIntersect(rayO, rayD, primitives[i]).x;
			if ((tBox > EPSILON) && (tBox < t))
			{
				t = tBox;
				return SHADOW;
			}
		}
		else if (primitives[i].id == --3) { /////-----CYLINDER-----|||||
			float tcylinder = cylinderIntersect(rayO, rayD, primitives[i]).x;
			if ((tcylinder > EPSILON) && (tcylinder < t))
			{
				t = tcylinder;
				return SHADOW;
			}
		}*/
		/*
		else if(primitives[i].id == --4){ /////-----PLANE-----|||||
			float tPlane = planeIntersect(rayO, rayD, primitives[i]);
			if((tPlane > EPSILON) && (tPlane < t)){
				t = tPlane;
				return SHADOW;
			}
		}
		else if(primitives[i].id == --5){ /////-----DISK-----|||||
			float tDisk = diskIntersect(rayO,rayD, primitives[i]);
			if((tDisk > EPSILON) && (tDisk < t)){
				t = tDisk;
				return SHADOW;
			}
		}*/
	//} 
	return 1.0;
}


#endif
