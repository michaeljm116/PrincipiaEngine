#include "../pch.h"
#include "debug-system.h"
#include "debug-component.hpp"
#include "../Rendering/imgui-renderer.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Principia {
	Sys_Debug::Sys_Debug()
	{
		addComponentType<Cmp_Debug>();
		addComponentType<NodeComponent>();
		addComponentType<TransformComponent>();
	}
	Sys_Debug::~Sys_Debug()
	{
	}
	void Sys_Debug::initialize()
	{
		debugMapper.init(*world);
		nodeMapper.init(*world);
		transformMapper.init(*world);
	}
	void Sys_Debug::begin()
	{
		io = ImGui::GetIO();
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		static bool show = true;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		//ImGui::OpenPopup("Debug Info");
	}
	void Sys_Debug::added(artemis::Entity& e)
	{
	}
	void Sys_Debug::processEntity(artemis::Entity& e)
	{
		auto* d = debugMapper.get(e);
		auto* n = nodeMapper.get(e);
		auto* t = transformMapper.get(e);

		
		ImGui::Text("Entity(%i): %s", e.getId(), n->name.c_str());
		ImGui::Text(d->message.c_str());
		ImGui::Text("Position X: %f, Y: %f, Z: %f", t->local.position.x, t->local.position.y, t->local.position.z);
		ImGui::Text("Rotation X: %f, Y: %f, Z: %f", t->eulerRotation.x, t->eulerRotation.y, t->eulerRotation.z);
		ImGui::Text("Quat_Rot X: %f, Y: %f, Z: %f, W: %f", t->local.rotation.x, t->local.rotation.y, t->local.rotation.z, t->local.rotation.w);
		ImGui::Text("Scale X: %f, Y: %f, Z: %f", t->local.scale.x, t->local.scale.y, t->local.scale.z);

		ImGui::Separator();
	}
	void Sys_Debug::removed(artemis::Entity& e)
	{
	}
	void Sys_Debug::end()
	{
		//ImGui::EndPopup();
	}
}