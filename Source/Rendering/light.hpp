#pragma once
/*Copyright (C) by Mike Murrell 
legacy*/
#include <glm\glm.hpp>
struct DirectionalLight
{
	//DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	glm::vec3 direction;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct PointLight
{
	//PointLight() { ZeroMemory(this, sizeof(this)); }

	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	// Packed into 4D vector: (Position, Range)
	glm::vec3 position;
	float Range;

	// Packed into 4D vector: (A0, A1, A2, Pad)
	glm::vec3 att;
	float pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct SpotLight
{
	//SpotLight() { ZeroMemory(this, sizeof(this)); }

	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;

	// Packed into 4D vector: (Position, Range)
	glm::vec3 position;
	float range;

	// Packed into 4D vector: (Direction, Spot)
	glm::vec3 direction;
	float spot;

	// Packed into 4D vector: (Att, Pad)
	glm::vec3 att;
	float pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct LightMaterial
{
	//Material() { ZeroMemory(this, sizeof(this)); }
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular; // w = SpecPower
	glm::vec4 reflect;
};


