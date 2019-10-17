#include <chrono>

namespace Principia {
	struct Timer {
		std::chrono::time_point<std::chrono::steady_clock> start;
		std::chrono::time_point<std::chrono::steady_clock> end;
		std::chrono::duration<float> duration;
		const char* name;
		float ms;

		Timer() {};
		Timer(const char* n) {
			name = n;
		};
		~Timer() {};
		void Start() {
			start = std::chrono::high_resolution_clock::now();
		};
		void End() {
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;

			ms = duration.count() * 1000.0f;
		};
	};

	struct NamedTimer {
		std::chrono::time_point<std::chrono::steady_clock> start;
		std::chrono::time_point<std::chrono::steady_clock> end;
		std::chrono::duration<float> duration;
		const char* name;
		float ms;

		NamedTimer(const char* n) {
			name = n;
			start = std::chrono::high_resolution_clock::now();
		};
		~NamedTimer() {
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;

			ms = duration.count() * 1000.0f;
			std::cout << name << ": " << ms << "ms \n";
		};
	};

}