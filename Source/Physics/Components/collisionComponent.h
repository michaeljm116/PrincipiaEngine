#pragma once
#include <Artemis/Component.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unordered_map>
#include <BulletDynamics/Dynamics/btRigidBody.h>

namespace Principia {
	enum class CollisionState {
		None, Enter, Continue, Exit
	};
	enum class CollisionType {
		Sphere = 0x00, 
		Box = 0x01, 
		Capsule = 0x02,
		Other = 0x03,
		Ghost = 0x04,
		Plane = 0x05
	};
	
	//This component is passed to any entity you wish to have collisions with
	struct CollisionComponent : artemis::Component {
		bool checked = false;
		int id = -1;
		btRigidBody* body = nullptr;
		float mass = 0.f;
		CollisionType type = CollisionType::Sphere;
		glm::vec3 extents = {};
		glm::vec3 position = {};
		glm::vec3 local = {};
		CollisionComponent() {};
		CollisionComponent(CollisionComponent* cc) : position(cc->position), local(cc->local), extents(cc->extents), type(cc->type),
													 mass(cc->mass), body(cc->body), id(cc->id), checked(cc->checked) {}
		CollisionComponent(glm::vec3 p, glm::vec3 l, glm::vec3 e, CollisionType t) : position(p),local(l), extents(e), type(t) { }
		CollisionComponent(glm::vec3 pos, glm::vec3 e, CollisionType t) : local(pos), extents(e), type(t){ }
		CollisionComponent(glm::vec4 pos, glm::vec4 e, CollisionType t) : local(glm::vec3(pos)), extents(glm::vec3(e)), type(t) {}
		CollisionComponent(glm::vec3 pos, glm::vec3 e, CollisionType t, float m) : local(pos), extents(e), type(t), mass(m) { }
	};

	//When you collide with an entity, this data is passed to it
	static const int max_timer = 5;
	struct CollisionData{
		int id = -1;
		int timer = max_timer - 1; //This is a timer to know how long the collision has been going on
		//int prev = 0;
		CollisionState state = CollisionState::None;
		glm::vec3 colpoint = {};
		glm::vec3 normal = {};
		CollisionData() {};
		CollisionData(int i) : id(i) {};
		CollisionData(int i, glm::vec3 c, glm::vec3 n) : id(i), colpoint(c), normal(n) {};
	};

	//This component is passed to an entity when it collides with it
	struct CollidedComponent : artemis::Component {
		std::vector<CollisionData> collidedWith;
		CollidedComponent(CollisionData d) {
			collidedWith.push_back(d);
		}
		void update(CollisionData cd) {
			//bool found = false;
			for (auto& cw : collidedWith) {
				if (cw.id == cd.id) {
					//cw.position = cd.position;
					//found = true;
					cw.normal = cd.normal;
					cw.colpoint = cd.colpoint;
					cw.timer = max_timer;
					cw.state = CollisionState::Continue;
					return;
				}
			}
			//TODOthiz iz irrelevant zince if it goe here it muzt not have found
			//if (!found) //This is a unique collision there should be an alert alz
				collidedWith.push_back(cd);
		}
	};

	//glm 2 bullet vector3
	inline btVector3 g2bv3(const glm::vec4& v) {
		return btVector3(v.x, v.y, v.z);
	}
	inline btVector3 g2bv3(const glm::vec3& v) {
		return btVector3(v.x, v.y, v.z);
	}
	//glm 2 bullet transform
	inline btTransform g2bt(const glm::mat4& t) {
		auto pos = g2bv3(t[3]);
		auto rot = btMatrix3x3(
			t[0][0], t[1][0], t[2][0],
			t[0][1], t[1][1], t[2][1],
			t[0][2], t[1][2], t[2][2]);
		return btTransform(rot, pos);
	}

	inline btQuaternion g2bq(const glm::quat q) {
		return btQuaternion(q.x, q.y, q.z, q.w);
	}

	inline glm::quat b2gq(const btQuaternion& q) {
		return glm::quat(q.w(), q.x(), q.y(), q.z());
	}
	inline glm::vec4 b2gv4(const btVector3 v) {
		return glm::vec4(v.x(), v.y(), v.z(), 1.f);
	}
	inline glm::vec3 b2gv3(const btVector3 v) {
		return glm::vec3(v.x(), v.y(), v.z());
	}
}