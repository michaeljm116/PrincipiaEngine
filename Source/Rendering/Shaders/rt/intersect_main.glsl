#ifndef _INTERSECT_MAIN_GLSL
#define _INTERSECT_MAIN_GLSL

#include "intersect_plane.glsl"
#include "intersect_box.glsl"
#include "intersect_sphere.glsl"
#include "intersect_mesh.glsl"
#include "intersect_cylinder.glsl"
#include "intersect_disk.glsl"

#define TRIINTERSECT true
#define SHADOW 0.2

const uint TYPE_SPHERE = 0x00000001u;
const uint TYPE_BOX = 0x00000002u;
const uint TYPE_PLANE = 0x00000004u;
const uint TYPE_MESH = 0x00000008u;
const uint TYPE_CYLINDER = 0x00000010u;
const uint TYPE_DISK = 0x00000020u;
const uint TYPE_JOINT = 0x00000040u;

sectID intersect(in vec3 rayO, in vec3 rayD, inout float resT, inout vec3 norm)
{
	sectID id = sectID(0, -1, -1);

	for (int i = 0; i < primitives.length(); ++i) {
		if (primitives[i].id > -1) {
			mat4 invWorld = inverse(primitives[i].world);
			vec3 rdd = (invWorld*vec4(rayD, 0.0)).xyz;// / primitives[i].extents;
			vec3 roo = (invWorld*vec4(rayO, 1.0)).xyz;// / primitives[i].extents;
			flool tMesh = boundsIntersect(roo, rdd, vec3(1, 1, 1));// primitives[i].extents);
			if (tMesh.b && (tMesh.t > EPSILON) && (tMesh.t < resT)) { //hits the boundingbox, doesnt necessarily mean tri hit
				//Mesh m = meshes[primitives[i].id];
				//id.pId = i;
				//rdd /= primitives[i].extents;
				//roo /= primitives[i].extents;
				int startIndex = primitives[i].startIndex;
				int endIndex = primitives[i].endIndex;
				for (int f = startIndex; f < endIndex; f++) {
					vec4 tQuad = quadIntersect(roo, rdd, faces[f]);
					if ((tQuad.x > 0) && (tQuad.x > EPSILON) && (tQuad.x < resT)) {
						id = sectID(TYPE_MESH, f, i);
						resT = tQuad.x;
						norm.x = tQuad.y;
						norm.y = tQuad.z;
					}
				}
			}
		}//id > 0

		else if (primitives[i].id == -1) { //SPHERE INTERSECT
			float tSphere = sphereIntersect(rayO, rayD, primitives[i]);
			if ((tSphere > EPSILON) && (tSphere < resT))
			{
				id = sectID(TYPE_SPHERE, i, -1);
				resT = tSphere;
			}
		}
		else if (primitives[i].id == -2) { //BOX INTERSECT
			vec4 tBox = boxIntersect(rayO, rayD, primitives[i]);
			if (tBox.x > 0) {
				if ((tBox.x > EPSILON) && (tBox.x < resT)) {
					id = sectID(TYPE_BOX, i, -1);
					resT = tBox.x;
					norm = tBox.yzw;
				}
			}
		}
		else if (primitives[i].id == -3) { //CYLINDER INTERSECT
			vec4 tCylinder = cylinderIntersect(rayO, rayD, primitives[i]);
			if ((tCylinder.x > EPSILON) && (tCylinder.x < resT)) {
				id = sectID(TYPE_CYLINDER, i, -1);
				resT = tCylinder.x;
				norm = tCylinder.yzw;
			}
		}
		else if (primitives[i].id == -4) { //PLANE INTERSECT
			float tplane = planeIntersect(rayO, rayD, primitives[i]);
			if ((tplane > EPSILON) && (tplane < resT))
			{
				id = sectID(TYPE_PLANE, i, -1);
				resT = tplane;
			}
		}
		else if (primitives[i].id == -5) { //DISK INTERSECT
			float tDisk = diskIntersect(rayO, rayD, primitives[i]);
			if ((tDisk > EPSILON) && (tDisk < resT)) {
				id = sectID(TYPE_DISK, i, -1);
				resT = tDisk;
			}
		}
	}

	//FOR SKELETAL ANIMATION
	for (int i = 0; i < joints.length(); ++i) {
		Joint j = joints[i];
		mat4 invWorld = inverse(j.world);
		vec3 rdd = (invWorld*vec4(rayD, 0.0)).xyz;
		vec3 roo = (invWorld*vec4(rayO, 1.0)).xyz;
		flool tMesh = boundsIntersect(roo, rdd, vec3(1, 1, 1));
		if (tMesh.b && (tMesh.t > EPSILON) && (tMesh.t < resT)) {

			for (int f = j.startIndex; f < j.endIndex; ++f) {
				vec4 tQuad = quadIntersect(roo, rdd, faces[f]);
				if ((tQuad.x > 0) && (tQuad.x > EPSILON) && (tQuad.x < resT)) {
					id = sectID(TYPE_JOINT, f, i);
					resT = tQuad.x;
					norm.x = tQuad.y;
					norm.y = tQuad.z;
				}
			}
			for (int s = j.startShape; s < j.endShape; ++s) {
				float tSphere = skinnedSphereIntersect(roo, rdd, shapes[s]);
				if ((tSphere > EPSILON) && (tSphere < resT)) {
					id = sectID(TYPE_SPHERE, s, -1);
					resT = tSphere;
				}
			}

		}

	}
	return id;
}

//rayO = pos, rayD = light, primitiveId = duh, t = t = length(lights[0].pos - pos);
float calcShadow(in vec3 rayO, in vec3 rayD, in sectID primitiveId, inout float t)
{
	for (int i = 0; i < primitives.length(); ++i) {
		if (primitives[i].id > -1) {/////-----MESH-----|||||
			mat4 invWorld = inverse(primitives[i].world);
			vec3 rdd = (invWorld*vec4(rayD, 0.0)).xyz;// / primitives[i].extents;
			vec3 roo = (invWorld*vec4(rayO, 1.0)).xyz;// / primitives[i].extents;

			flool tMesh = boundsIntersect(roo, rdd, vec3(1, 1, 1));// primitives[i].extents);
			if (tMesh.b && (tMesh.t > EPSILON) && (tMesh.t < t)) {
				int startIndex = primitives[i].startIndex;
				int endIndex = primitives[i].endIndex;
				for (int j = startIndex; j < endIndex; j++) {
					vec3 normal = vec3(0, 1, 0);
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
		else if (primitives[i].id == -1) { /////-----SPHERE-----|||||
			float tSphere = sphereIntersect(rayO, rayD, primitives[i]);
			if ((tSphere > EPSILON) && (tSphere < t)) {
				t = tSphere;
				return SHADOW;
			}
		}
		else if (primitives[i].id == -2) { /////-----BOX-----|||||
			float tBox = boxIntersect(rayO, rayD, primitives[i]).x;
			if ((tBox > EPSILON) && (tBox < t))
			{
				t = tBox;
				return SHADOW;
			}
		}
		else if (primitives[i].id == -3) { /////-----CYLINDER-----|||||
			float tcylinder = cylinderIntersect(rayO, rayD, primitives[i]).x;
			if ((tcylinder > EPSILON) && (tcylinder < t))
			{
				t = tcylinder;
				return SHADOW;
			}
		}
		/*
		else if(primitives[i].id == -4){ /////-----PLANE-----|||||
			float tPlane = planeIntersect(rayO, rayD, primitives[i]);
			if((tPlane > EPSILON) && (tPlane < t)){
				t = tPlane;
				return SHADOW;
			}
		}
		else if(primitives[i].id == -5){ /////-----DISK-----|||||
			float tDisk = diskIntersect(rayO,rayD, primitives[i]);
			if((tDisk > EPSILON) && (tDisk < t)){
				t = tDisk;
				return SHADOW;
			}
		}*/
	} 
	return 1.0;
}


#endif
