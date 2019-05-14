#pragma once
#include <vector>
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

