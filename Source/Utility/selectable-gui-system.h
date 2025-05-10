#pragma once
#include "Artemis/EntityProcessingSystem.h"
namespace Principia {

	struct Cmp_Selectable;
	struct GUIComponent;


	class Sys_Selectable_GUI : public artemis::EntityProcessingSystem {
	public:
		Sys_Selectable_GUI();
		~Sys_Selectable_GUI();

		void initialize() override;
		void processEntity(artemis::Entity& e) override;
		void removed(artemis::Entity& e) override;
		void update();

		glm::ivec2 ConvertUnitToPixel(const glm::vec2& percentage);

	private:
		artemis::ComponentMapper<Cmp_Selectable> selectable_mapper;
		artemis::ComponentMapper<GUIComponent> gui_mapper;

		//TODO: this should be a vector or map for multiple selects, not needed for this game though
		artemis::Entity* selected_entity; 

	};

}