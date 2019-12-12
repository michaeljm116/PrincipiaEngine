#pragma once

#include "Artemis/Component.h"
#include "Artemis/Entity.h"
#include <array>
#include <iostream>
namespace Principia {
	struct GridComponent : public artemis::Component {
		std::array<std::array<artemis::Entity*, 16>, 16> grid;
		struct GridSize {
			uint16_t x : 8;
			uint16_t y : 8;
		}size;

		GridComponent(uint16_t x, uint16_t y) { 
			size.x = x; 
			size.y = y;
			for (int i = 0; i < 16; ++i) {
				for (int j = 0; j < 16; ++j)
					grid[i][j] = nullptr;
			}
		};
	};

	struct GridBlock {
		int leftx;
		int rightx;
		int downy;
		int upy;

		GridBlock(const glm::vec3& position, const glm::vec3& extents) {
			leftx = int(position.x - extents.x) >> 1;
			rightx = int(round(position.x + extents.x) * 0.5f);
			downy = int(position.z - extents.z) >> 1;
			upy = int(round(position.z + extents.z) * 0.5f);
		}
		bool verify(const GridComponent::GridSize& s) {
			if ((leftx < 0) || (downy < 0) || (rightx > s.x) || (upy > s.y)) {
				std::cout << "ERROR OUT OF BOUNDS";
				return false;
			}
			return true;
		}
	};
}