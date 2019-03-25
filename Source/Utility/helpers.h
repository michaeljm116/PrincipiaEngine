#pragma once
namespace tulip {
	template<typename T>
	inline T max(const T& a, const T& b) {
		if (a > b) return a; else return b;
	};

	template <typename T>
	inline T min(const T& a, const T& b) {
		if (a < b) return a; else return b;
	} 
}

