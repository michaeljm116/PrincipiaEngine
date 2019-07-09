#ifndef _STRUCTS_GLSL
#define _STRUCTS_GLSL

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

struct Primitive
{
	mat4 world;
	//mat4 invWorld;
	vec3 extents;
	int numChildren;
	int id;
	int matID;
	int startIndex;
	int endIndex;
};

struct Joint {
	mat4 world;
	vec3 extents;
	int id;	

	int startIndex;
	int endIndex;
	int startShape;
	int endShape;
}; 

struct TriangleVert{
	vec3 pos;
	int _pad;
};

struct TriangleIndex{
	int v0;
	int v1;
	int v2;
	int id;
};

struct Vert {
	vec3 pos;
	float u;
	vec3 norm;
	float v;
};
struct Face {
	ivec4 v;
};

struct Shape{
	vec3 center;
	int matID;
	vec3 extents;
	int type;
};

struct Material{
	vec3 diffuse;
	float reflective;

	float roughness;
	float transparency;
	float refractiveIndex;
	int textureID;
};

struct Light{
	vec3 pos;
	float intensity;
	vec3 color;
	int id;
};

struct Gui {
	vec2 min;
	vec2 extents;
	vec2 alignMin;
	vec2 alignExt;
	int layer;
	int id;
	int pad;
	int visible;
};

struct flool{
	float t;
	bool b;
};

struct boxReturn{
	float t;
	vec3 tdd;
	float _pad;
	vec3 t1;
};

struct finalmaterial{
	vec3 color;
	float reflection;

	float roughness;
	float transparency;
	float refraction;
	float _pad;
};

struct sectID{
	uint type;
	int id;
	int pId;
};


#endif