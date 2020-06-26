#include "timer.h"

namespace Principia {
	Timer::Timer(const char* n) {
		name = n;
	}
	Timer::~Timer() {}
	void Timer::Start() {
		start = std::chrono::high_resolution_clock::now();
	}
	void Timer::End() {
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		ms = duration.count() * 1000.0f;
	}
}