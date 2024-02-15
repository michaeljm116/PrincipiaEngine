#pragma once


#include "../Utility/transformComponent.hpp"
#include "../Utility/nodeComponent.hpp"
#include "../Utility/xxhash.hpp"

#define ANIM_FLAG_RESET 0b11111111111000000000000000000000
namespace Principia {
	
	//Returns a Breadth-First Vector of the nodes;
	inline void flatten(std::vector<NodeComponent*>& vec, NodeComponent* head) {
		std::queue<NodeComponent*> q;
		q.push(head);
		while (!q.empty()) {
			auto f = q.front();
			q.pop();
			for (auto child : f->children) {
				q.push(child);
				vec.push_back(child);
			}
		}
	}


	//Breadth First Graph Component
	//An array of node pointers layed out in BFS order
	struct BFGraphComponent : artemis::Component {
		std::vector<NodeComponent*> nodes;
		std::vector<sqt> transforms;
	};

	inline void flatten(BFGraphComponent* graph, NodeComponent* head) {
		std::queue<NodeComponent*> q;
		q.push(head);
		while (!q.empty()) {
			auto f = q.front();
			q.pop();
			for (auto child : f->children) {
				auto transform = ((TransformComponent*)child->data->getComponent<TransformComponent>())->local;
				q.push(child);
				graph->nodes.push_back(child);
				graph->transforms.push_back(transform);
			}
		}
	}

	// Animation flags 4 bytes
	// pf, rf, sf = positionflag, rotationflag, scaleflag
	// I thinks to make sure you only animate the things you want to animate?
	struct AnimFlags {
		uint32_t idPo : 8;
		uint32_t loop : 1;
		uint32_t forceStart : 1;
		uint32_t forceEnd : 1;
		uint32_t pf : 1;
		uint32_t rf : 1;
		uint32_t sf : 1;
		uint32_t startSet : 1;
		uint32_t endSet : 1;

		AnimFlags() { idPo = 0; loop = 0; forceStart = 0; forceEnd = 0; startSet = 0; endSet = 0; _pad = 0; };
		//id, loop 1 = true, forcestart 1 = true, force end 1 = true
		AnimFlags(int i, int l, int fs, int fe) {
			idPo = i; loop = l; forceStart = fs; forceEnd = fe;
			startSet = 0; endSet = 0;
		}
	private:
		uint32_t _pad : 16; //O snap i can just make this 16 bits?
	};

	struct AnimateComponent : artemis::Component {
		float currTime = 0.f;
		float time = 1.f;
		AnimFlags flags;

		sqt start;
		sqt end;
		AnimateComponent() {};
	};

	struct PoseComponent : artemis::Component {
		std::vector<std::pair<int, sqt>> pose;
		std::string fileName;
		std::string poseName;

		PoseComponent(std::string n, std::string f, std::vector<std::pair<int,sqt>> p) :
			poseName(n), fileName(f), pose(p) {};
	};

	struct AnimationComponent : artemis::Component {
		int num_poses = 0;
		AnimFlags flags = AnimFlags(0, 0, 0, 0);

		float time = 1.005f;
		int start = 0;
		int end = 0;
		int prefabName = 0;

		float transTime = 0;
		int trans = 0;
		int transEnd = 0;

		AnimationComponent(int n, std::string&& p, std::string&& s, std::string&& e, AnimFlags f) :
			num_poses(n), flags(f) {
			prefabName = xxh::xxhash<32, char>(p.c_str());
			start = xxh::xxhash<32, char>(s.c_str());
			end = xxh::xxhash<32, char>(e.c_str());
		};
		AnimationComponent(int n, std::string&& p, std::string&& e, AnimFlags f) :
			num_poses(n), flags(f) {
			prefabName = xxh::xxhash<32, char>(p.c_str());
			start = 0;
			end = xxh::xxhash<32, char>(e.c_str());
		}
		AnimationComponent(int n, const int p, const int e, AnimFlags f) :
			num_poses(n), prefabName(p), end(e), flags(f) {};
		AnimationComponent(int n, const int p, const int s, const int e, AnimFlags f) :
			flags(f) {
			num_poses = n; prefabName = p; start = s; end = e;
		};

		AnimationComponent(float t, const int p, const int s, const int e, AnimFlags f) :
			flags(f) {
			num_poses = 2; prefabName = p; start = s; end = e; time = t;
		};
		AnimationComponent() {};
	};
}