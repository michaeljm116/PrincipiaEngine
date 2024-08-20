#include "../pch.h"
#include "selectable-gui-system.h"
#include "../Rendering/Components/renderComponents.hpp"
#include "input.h"
#include "window.h"

Principia::Sys_Selectable_GUI::Sys_Selectable_GUI()
{
	addComponentType<Cmp_Selectable>();
	addComponentType<GUIComponent>();
}

Principia::Sys_Selectable_GUI::~Sys_Selectable_GUI()
{
}

void Principia::Sys_Selectable_GUI::initialize()
{
	selectable_mapper.init(*world);
	gui_mapper.init(*world);
}

void Principia::Sys_Selectable_GUI::processEntity(artemis::Entity& e)
{
	auto* selected = selectable_mapper.get(e);
	if (selected != nullptr && selected->active) {
		const auto* gui = gui_mapper.get(e);
		if (selected == nullptr)
			return;
		const glm::ivec2 min = ConvertUnitToPixel(gui->min);
		const glm::ivec2 extents = ConvertUnitToPixel(gui->extents);
		const glm::ivec2 mouse = glm::ivec2(pINPUT.mouse.x, pINPUT.mouse.y);
		const glm::ivec2 diff = -min + glm::ivec2(pINPUT.mouse.x, WINDOW.getHeight() - pINPUT.mouse.y);
		bool hit = (diff.x > 0 && diff.x < extents.x) && (diff.y > 0 && diff.y < extents.y);
		if (hit) {
			selected->state = (SelectableState)pINPUT.mouse.buttons[GLFW_MOUSE_BUTTON_LEFT];
			//selected_entity = &e;
		}
	}
}

void Principia::Sys_Selectable_GUI::removed(artemis::Entity& e)
{
	if (selected_entity == &e)
		selected_entity = nullptr;
}

void Principia::Sys_Selectable_GUI::update()
{
	if (pINPUT.mouse.buttons[GLFW_MOUSE_BUTTON_LEFT] & MOUSE_BUTTON_CHANGED) {
		process();
	}
}

glm::ivec2 Principia::Sys_Selectable_GUI::ConvertUnitToPixel(const glm::vec2& percentage)
{
	int x = int(percentage.x * WINDOW.getWidth());
	int y = int(percentage.y * WINDOW.getHeight());
	return glm::ivec2(x, y);
}
