#pragma once
#ifndef COLLISIONCOMPONENTS_HPP
#define COLLISIONCOMPONENTS_HPP


#include <Artemis/Artemis.h>
#include <glm/glm.hpp>
#include <unordered_map>

struct BBComponent {
	glm::vec3 center;
};

struct AABBComponent : public BBComponent, public artemis::Component
{
	//glm::vec3 center;
	glm::vec3 extents;

	AABBComponent() {};
	AABBComponent(glm::vec3 c, glm::vec3 e) : extents(e) { center = c; };
};

struct SBBComponent : public BBComponent, public artemis::Component
{
	//glm::vec3 center;
	float radius;

	SBBComponent() {};
	SBBComponent(glm::vec3 c, float r) : radius(r) { center = c; };
};

struct RigidBodyComponent : public artemis::Component {

	//btScalar mass;
	//btTransform trans;
	//int coaIndex;
	//bool impulse = false;
	//bool toggled = false;
	//btRigidBody* body;
	//
	//RigidBodyComponent() {};
	//RigidBodyComponent(float m, glm::mat4 t) {
	//	mass = btScalar(m);
	//	btMatrix3x3 rotation = 
	//	btMatrix3x3(
	//		t[0][0], t[1][0], t[2][0],
	//		t[0][1], t[1][1], t[2][1],
	//		t[0][2], t[1][2], t[2][2]);
	//	btVector3 position = btVector3(t[3][0], t[3][1], t[3][2]);
	//	//btVector3 position = btVector3(t[0][3], t[1][3], t[2][3]);
	//	trans = btTransform(rotation, position);
	//}
};

/*
sokay so the way it is right now... its already on collisoin repeat
and even when there's no way of knowing when you start a colliion and when u stop
so the question is how do you know if you start?? 
hash of starts is a good way
col state is another way???


*/
enum class CollisionState {
	Start, Continue, Exit
};
struct CollisionData {
	//btVector3 pos;
	//btVector3 norm;
	//CollisionState state;
	//CollisionData(const btVector3& p, const btVector3& n) : pos(p), norm(n) { state = CollisionState::Start; };
	//CollisionData() {};
};
struct CollisionComponent : public artemis::Component {
	std::unordered_map<int, CollisionData> collisions; 
};

struct SpringComponent : public artemis::Component {
	//btVector3 normal;
	//float power;
	//SpringComponent(glm::vec3 norm, float pow) : normal(btVector3(norm.x, norm.y, norm.z)), power(pow) {};
	//SpringComponent(btVector3 norm, float pow) : normal(norm), power(pow) {};

};

struct ImpulseComponent : public artemis::Component {
	//btVector3 direction;
	//float force;

	//ImpulseComponent(glm::vec3 dir, float f) : direction(btVector3(dir.x, dir.y, dir.z)), force(f) {};
	//ImpulseComponent(btVector3 dir, float f) : direction(dir), force(f) {};
};

#endif // !COLLISIONCOMPONENTS.HPP
