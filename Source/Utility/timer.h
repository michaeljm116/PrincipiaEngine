#pragma once
#include <chrono>

namespace Principia {
	class Timer {
	private:
		std::chrono::time_point<std::chrono::steady_clock> start;
		std::chrono::time_point<std::chrono::steady_clock> end;
		std::chrono::duration<float> duration;

	public:
		float ms;
		const char* name;

		Timer(const char* n);
		~Timer();
		void Start();
		void End();
	};



}