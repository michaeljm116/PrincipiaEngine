#pragma once
/*
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

	
	
	//0 1 2 3
	//1
	//2
	//3

	//  1 2 3
	//3
	//2
	//1
	 
	
	struct GridBlock {
		int leftx : 8;
		int rightx : 8;
		int downy : 8;
		int upy : 8;

		const int itr = 1;  //This makes it so you iterate by a certain factor
							//since the extents goes from -1 to 1 its 2
		GridBlock(const glm::vec3& position, const glm::vec3& extents) {
			leftx = int(position.x - extents.x) >> 1;
			rightx = int(round(position.x + extents.x) * 0.5f);
			upy = 16 - (int(position.z - extents.z) >> 1);
			downy = 16 - int(round(position.z + extents.z) * 0.5f);
		}
		bool verify(const GridComponent::GridSize& s) {
			//if ((leftx < 0) || (downy < 0) || (rightx > s.x) || (upy > s.y)) {
			//	std::cout << "ERROR OUT OF BOUNDS";
			//	return false;
			//}
			if (leftx < 0) leftx = 0;
			if (downy < 0) downy = 0;
			if (rightx > s.x) rightx = s.x;
			if (upy > s.y) upy = s.y;
			return true;
		}
	};
}*/