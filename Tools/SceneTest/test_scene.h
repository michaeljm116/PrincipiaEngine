#pragma once
#include <Artemis/Artemis.h>
#include <vector>
#include "Utility/nodeComponent.hpp"

namespace test {
	class TestScene {
	public:
		TestScene(std::string&& s);
		~TestScene();

		void LoadScene(std::string s);

		inline const std::vector<Principia::NodeComponent*> GetParents() { return parents; }
	private:
		std::vector<Principia::NodeComponent*> parents;

	};
}