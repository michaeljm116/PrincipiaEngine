#ifndef _STRUCTS_GLSL
#define _STRUCTS_GLSL

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

struct Ray {
	vec3 o;
	float t;
	vec3 d;
	float t2;
};

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

struct BVHNode {
	vec3 upper;
	int offset;
	vec3 lower;
	int numChildren;
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
	//vec3 scale;
	//int type;
};

struct Gui {
	vec2 min;
	vec2 extents;
	vec2 alignMin;
	vec2 alignExt;
	int layer;
	int id;
	int pad;
	float alpha;
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

struct iSectID{
	uint primType;
	int  faceID;
	int  primID;
	int  nodeID;
};

struct HitInfo
{
    float t;
    vec3 normal;

    uint prim_type;
    int prim_id;
    int face_id;
    int node_id;
};


#endif
