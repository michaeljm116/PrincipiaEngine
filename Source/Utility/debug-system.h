#pragma once
/* This system iterates over all debug components
	And renders them on ImGui
*/

#include "debug-component.hpp"
#include "nodeComponent.hpp"
#include "transformComponent.hpp"
#include "../Rendering/imgui-renderer.h"
#include "Artemis/EntityProcessingSystem.h"
#include "Artemis/ComponentMapper.h"

namespace Principia {

	class Sys_Debug : public artemis::EntityProcessingSystem
	{
	public:
		Sys_Debug();
		~Sys_Debug();
		void initialize() override;
		void begin();
		void added(artemis::Entity& e) override;
		void processEntity(artemis::Entity& e) override;
		void removed(artemis::Entity& e) override;
		void end();

		inline void setImGui(ImGuiRenderer* imgui) { this->imgui = imgui; }
	private:
		artemis::ComponentMapper<Cmp_Debug> debugMapper = {};
		artemis::ComponentMapper<NodeComponent> nodeMapper = {};
		artemis::ComponentMapper<TransformComponent> transformMapper = {};
		ImGuiRenderer* imgui = nullptr;
		ImGuiIO io = {};
	};

}