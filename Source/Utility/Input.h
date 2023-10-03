#pragma once
/* Copyright (C) Mike Murrell 2017 Input class
Handles all the inputs yo
Global class that updates input if its done
note: after watching the overwatch GDC lecture...
i should probably make it more ecs friendly and 
just have an input system, but ill put that on the todolist
*/
#ifndef INPUT_H
#define INPUT_H

#define UIIZON
#include "window.h"
#include "../Utility/key.h"
#include "../Utility/mouse.h"

#include "../imgui/imgui.h"

namespace Principia {

#define pINPUT Input::get()

	class Input
	{
	public:

		static Input& get() {
			static Input instance;
			return instance;
		}
	private:
		Input() {};
		~Input();

		GLFWwindow* window;

		void KeyDirection(int key, bool pressed);

		static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
		{
			pINPUT.keys[key] = action;
			switch (action)
			{
			case GLFW_PRESS:
				pINPUT.pressed = true;
				break;
			case GLFW_REPEAT:
				pINPUT.pressed = true;
				break;
			case GLFW_RELEASE:
				pINPUT.pressed = false;
				break;
			default:
				break;
			}

#ifdef UIIZON
			ImGuiIO& io = ImGui::GetIO();
			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				io.KeysDown[key] = true;
			}
			else if (action == GLFW_RELEASE) {
				io.KeysDown[key] = false;
			}
#endif // 


		}

		static void char_callback(GLFWwindow*, unsigned int c)
		{
#ifdef UIIZON
			ImGuiIO& io = ImGui::GetIO();
			io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
			io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
			if (c > 0 && c < 0x10000)
				io.AddInputCharacter((unsigned short)c);
#endif // UIIZON
		}

		static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
			pINPUT.mouse.updatePosition((int)xpos, (int)ypos);
		}


		//static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
		//	/*if (action == GLFW_PRESS || action == GLFW_REPEAT)
		//		INPUT.mouse.updateButton(button, true);
		//	else
		//		INPUT.mouse.updateButton(button, false);*/
		//	INPUT.mouse.buttons[button] = action;
		//	//INPUT.mouse.updateButton(button, (bool)action);
		//}

		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
			pINPUT.mouse.updateScroll(yoffset);
		}

		static void joystick_callback(int jid, int event) {
			if (event == GLFW_CONNECTED)
			{
				// The joystick was connected
			}
			else if (event == GLFW_DISCONNECTED)
			{
				// The joystick was disconnected
			}
		}


	public:
		void init();
		void update();

		//bool up, down, left, right;
		//bool pressed, held, released;
		Mouse mouse = Mouse();
		int keys[348] = {};
		int pad[8][14] = {};

		float deltaTime = 0.f;
		float time = 0.f;
		float renderTime = 0.f;
		bool pressed = false;
		bool hold = false;
		bool showFPS = false;
		bool maximized = false;
		bool displayUI = false;
		bool playToggled = false;
		bool playMode = false;

		bool hasGamepad = false;
	};

#endif // !INPUT_H
}