#pragma once
#include <vector>
#include <random>
namespace tulip {
	template<typename T>
	inline T max(const T& a, const T& b) {
		if (a > b) return a; else return b;
	};

	template <typename T>
	inline T min(const T& a, const T& b) {
		if (a < b) return a; else return b;
	} 

	template <typename T>
	inline T maxV(const T& a, const T& b) {
		T res;
		for (int i = 0; i < a.length(); ++i) {
			a[i] > b[i] ? res[i] = a[i] : res[i] = b[i];
		}

		return res;
	}

	template <typename T>
	inline T minV(const T& a, const T& b) {
		T res;
		for (int i = 0; i < a.length(); ++i) {
			a[i] < b[i] ? res[i] = a[i] : res[i] = b[i];
		}

		return res;
	}
}

namespace Principia {
	//template <typename T>
	inline int sign(const float& a) {
		return  1 | ((int)a >> (sizeof(int) * CHAR_BIT - 1));  // if v < 0 then -1, else +1
	}

	inline uint64_t random_int() {
		//static std::uniform_real_distribution<double> distribution(0.0, 1.0);
		static std::uniform_int_distribution<int32_t> distribution(INT_MIN, INT_MAX);
		static std::mt19937 generator;
		return distribution(generator);
	}

}

