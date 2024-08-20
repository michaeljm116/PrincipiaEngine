#pragma once
#include <chrono>

namespace Principia {
	class Timer {
	public:

		Timer(const char* n) : m_name(n) {};
		~Timer() {};
		inline void start() {
			m_start = std::chrono::high_resolution_clock::now();
		}
		inline void end() {
			m_end = std::chrono::high_resolution_clock::now();
			m_duration = m_end - m_start;
			m_milisecs = m_duration.count() * 1000.0f;
		}
		inline float get_msecs() const { return m_milisecs; }

	private:
		std::chrono::time_point<std::chrono::steady_clock> m_start = {};
		std::chrono::time_point<std::chrono::steady_clock> m_end = {};
		std::chrono::duration<float> m_duration = {};
		float m_milisecs = 0;
		const char* m_name;
	};
}