#pragma once
#include <Artemis/Artemis.h>
#include <functional>
#include <random>
#include <chrono>

#include "transformComponent.hpp"
#include "nodeComponent.hpp"
#include "../Physics/Components/collisionComponent.h"

namespace Principia {

	class Script {
	public: 
		Script() {};
		~Script() {};
		virtual void added() = 0;
		virtual void process(float dt) = 0;
		virtual void removed() = 0;
		inline static artemis::World* world;

	protected:
		artemis::Entity* entity = nullptr;
	};

	class CollisionScript : public Script {
	public: 
		CollisionScript() {};
		~CollisionScript() {};
		virtual void OnCollisionEnter(CollidedComponent* cc) = 0;
		virtual void OnCollisionStay(float dt) = 0;
		virtual void OnCollisionExit(artemis::Entity* e) = 0;
	protected:
		CollidedComponent* collision = nullptr;
	};

	class WorldScript : public Script {
	private:
		WorldScript() {};
	public:
		~WorldScript() {};
		static WorldScript& get() {
			static WorldScript instance;
			return instance;
		}
		WorldScript(WorldScript const&) = delete;
		void operator= (WorldScript const&) = delete;
		void added() override {};
		void process(float dt) override {};
		void removed() override {};
		void SetWorld(artemis::World& w) { world = &w; }
	};

	class TestScript : public Script {
	public:
		void added() { std::cout << "Added"; }
		void process(float dt) {
			std::cout << "\nProcessing";
			auto tc = (TransformComponent*)entity->getComponent<TransformComponent>();
			std::cout << tc->world[3][0];
		};
		void removed() { std::cout << "removed"; };
		TestScript(artemis::Entity* e) { entity = e; }
	};

	struct ScriptComponent : artemis::Component {
		std::function<void(void)> added = nullptr;
		std::function<void(float)> process = nullptr;
		std::function<void(void)> removed = nullptr;
		std::unique_ptr<Script> script = nullptr;

		//ScriptComponent(Script* s){
		//	script = s;
		//	added = [this]() {script->added(); };
		//	process = [this](float dt) {script->process(dt); };
		//	removed = [this]() {script->removed(); };
		//};
		ScriptComponent(std::unique_ptr<Script> s) {
			script = std::move(s);
			added = [this]() {script->added(); };
			process = [this](float dt) {script->process(dt); };
			removed = [this]() {script->removed(); };
		}
	};

	struct Cmp_Collision_Script : artemis::Component 
	{
		std::function<void(void)> added = nullptr;
		std::function<void(float)> process = nullptr;
		std::function<void(void)> removed = nullptr;

		std::function<void(CollidedComponent*)> OnCollisionEnter = nullptr;
		std::function<void(float)> OnCollisionStay = nullptr;
		std::function<void(artemis::Entity*)> OnCollisionExit = nullptr;

		std::unique_ptr<CollisionScript> script;

		//ScriptComponent(Script* s){
		//	script = s;
		//	added = [this]() {script->added(); };
		//	process = [this](float dt) {script->process(dt); };
		//	removed = [this]() {script->removed(); };
		//};
		Cmp_Collision_Script(std::unique_ptr<CollisionScript> s) 
		{
			script = std::move(s);
			added = [this]() {script->added(); };
			process = [this](float dt) {script->process(dt); };
			removed = [this]() {script->removed(); };

			OnCollisionEnter = [this](CollidedComponent* cc) {script->OnCollisionEnter(cc); };
			OnCollisionStay = [this](float dt) {script->OnCollisionStay(dt); };
			OnCollisionExit = [this](artemis::Entity* e) {script->OnCollisionExit(e); };
		}
	};

	struct HeartScript : public Script {
	public:
		void added() override {};
		void process(float dt) override {
			time += dt;
			auto* nc = (NodeComponent*)entity->getComponent<NodeComponent>();
			auto* tc = (TransformComponent*)nc->children[0]->data->getComponent<TransformComponent>();
			tc->eulerRotation.z = std::fma(30.f, dt, tc->eulerRotation.z);
			glm::mat4 rotM;
			rotM = glm::rotate(rotM, glm::radians(tc->eulerRotation.x), glm::vec3(1.f, 0.f, 0.f));
			rotM = glm::rotate(rotM, glm::radians(tc->eulerRotation.y), glm::vec3(0.f, 1.f, 0.f));
			rotM = glm::rotate(rotM, glm::radians(tc->eulerRotation.z), glm::vec3(0.f, 0.f, 1.f));

			tc->local.rotation = glm::toQuat(rotM);
			tc->local.position.y = std::fma(0.02f, glm::sin(time), tc->local.position.y);
		};
		void removed() override {};
		
		HeartScript(artemis::Entity* e) { entity = e; }
		HeartScript() {};
		~HeartScript() {}
	private: 
		float time = 0.f;
	};

	struct LightScript : public Script {
		//X & Z = 2 - 14, Y = -4 - 8
		LightScript(artemis::Entity* e) { entity = e; };
		LightScript() {};
		~LightScript() {};
		void added() override {};
		void process(float dt) override {
			time += dt;
			if (time > maxTime) {
				time = 0;
				std::default_random_engine generator;// std::chrono::system_clock::now());
				std::uniform_real_distribution<float> dx(2, 14);
				std::uniform_real_distribution<float> dz(2, 14);
				//std::uniform_real_distribution<float> dy(-4, 0);

				unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
				generator.seed(seed);
				nextPos.x = dx(generator);
				//nextPos.y = dy(generator);
				nextPos.z = dz(generator);

			}
			auto* tc = (TransformComponent*)entity->getComponent<TransformComponent>();
			glm::vec4 dtv = glm::vec4(dt * 0.25f);
			tc->local.position = glm::mix(tc->local.position, nextPos, dtv);
			
		};
		void removed() override {};

	private:
		float time = 0.f;
		float maxTime = 4.f;
		glm::vec4 nextPos = glm::vec4(8.f, -2.f, 8.f, 1.f);
	};

}
