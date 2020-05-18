#pragma once
#include "../Utility/transformComponent.hpp"
#include "../Utility/nodeComponent.hpp"
#include "../Utility/xxhash.hpp"

#define ANIM_FLAG_RESET 0b11111111111000000000000000000000
namespace Principia {
	
	//Animation flags 4 bytes
	struct AnimFlags {
		uint32_t id : 8;
		uint32_t loop : 1;
		uint32_t forceStart : 1;
		uint32_t forceEnd : 1;
		uint32_t pf : 1;
		uint32_t rf : 1;
		uint32_t sf : 1;
		uint32_t startSet : 1;
		uint32_t endSet : 1;

		AnimFlags() {};
		//id, loop 1 = true, forcestart 1 = true, force end 1 = true
		AnimFlags(int i, int l, int fs, int fe) {
			id = i; loop = l; forceStart = fs; forceEnd = fe;
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
		std::vector<std::pair<sqt, int>> pose;
		std::string fileName;
		std::string poseName;

		PoseComponent(std::string n, std::string f, std::vector<std::pair<sqt, int>> p) :
			poseName(n), fileName(f), pose(p) {};
	};

	struct AnimationComponent : artemis::Component {
		int num;
		AnimFlags flags;
		float time = .25f;
		int start;
		int end;
		int prefabName;

		AnimationComponent(int n, std::string&& p, std::string&& s, std::string&& e, AnimFlags f) :
			num(n), flags(f) {
			prefabName = xxh::xxhash<32, char>(p.c_str());
			start = xxh::xxhash<32, char>(s.c_str());
			end = xxh::xxhash<32, char>(e.c_str());
		};
		AnimationComponent(int n, std::string&& p, std::string&& e, AnimFlags f) :
			num(n), flags(f) {
			prefabName = xxh::xxhash<32, char>(p.c_str());
			start = 0;
			end = xxh::xxhash<32, char>(e.c_str());
		}
		AnimationComponent(int n, const int p, const int e, AnimFlags f) :
			num(n), prefabName(p), end(e), flags(f) {};
		AnimationComponent(int n, const int p, const int s, const int e, AnimFlags f) :
			flags(f) {
			num = n; prefabName = p; start = s; end = e;
		};

	};
}