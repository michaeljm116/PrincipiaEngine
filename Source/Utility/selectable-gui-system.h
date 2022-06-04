#pragma once

#include "../Rendering/Components/renderComponents.hpp"
#include "Input.h"

namespace Principia {
	class Sys_Selectable_GUI : public artemis::EntityProcessingSystem {
	public:
		Sys_Selectable_GUI();
		~Sys_Selectable_GUI();

		void initialize() override;
		void processEntity(artemis::Entity& e) override;
		void removed(artemis::Entity& e) override;
		void update();

	private:
		artemis::ComponentMapper<Cmp_Selectable> selectable_mapper;
		artemis::ComponentMapper<GUIComponent> gui_mapper;

		//TODO: this should be a vector or map for multiple selects, not needed for this game though
		artemis::Entity* selected_entity; 

		inline glm::ivec2 ConvertUnitToPixel(const glm::vec2& percentage) {
			int x = int(percentage.x * WINDOW.getWidth());
			int y = int(percentage.y * WINDOW.getHeight());
			return glm::ivec2(x, y);
		}
	};

}