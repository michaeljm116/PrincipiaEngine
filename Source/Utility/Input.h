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

#include "window.h"
#include "../Utility/key.h"
#include "../Utility/mouse.h"

#include "../imgui/imgui.h"

namespace Principia {

#define INPUT Input::get()
#define UIIZON

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

		static void Input::key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
		{
			/*if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				Input::get().pressed = true;
				Input::get().updateAxis(key, true);
				//Input::get().Keys[7].update(key == GLFW_KEY_SPACE);

			}
			else {
				Input::get().pressed = false;
				Input::get().updateAxis(key, false);
			}
			if (action == GLFW_RELEASE) {
				//Input::get().Keys[7].update(key == GLFW_KEY_SPACE);
				if (key == GLFW_KEY_P) {
					Input::get().playToggled = true;
					Input::get().playMode = !Input::get().playMode;
				}
			}
			if (key == GLFW_KEY_ESCAPE) {
				glfwSetWindowShouldClose(WINDOW.getWindow(), 1);
			}
			if (key == GLFW_KEY_F11) {
				WINDOW.toggleMaximized();
			}*/
			INPUT.keys[key] = action;
			switch (action)
			{
			case GLFW_PRESS:
				INPUT.pressed = true;
				break;
			case GLFW_REPEAT:
				INPUT.pressed = true;
				break;
			case GLFW_RELEASE:
				INPUT.pressed = false;
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
			if (c > 0 && c < 0x10000)
				io.AddInputCharacter((unsigned short)c);
#endif // UIIZON
		}

		static void Input::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
			INPUT.mouse.updatePosition((int)xpos, (int)ypos);
		}


		static void Input::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
			/*if (action == GLFW_PRESS || action == GLFW_REPEAT)
				INPUT.mouse.updateButton(button, true);
			else
				INPUT.mouse.updateButton(button, false);*/
			INPUT.mouse.buttons[button] = action;
		}

		static void Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
			INPUT.mouse.updateScroll(yoffset);
		}

		static void Input::joystick_callback(int jid, int event) {

		}


	public:
		void init();
		void update();
		void updateAxis(int key, bool pressed);

		//bool up, down, left, right;
		//bool pressed, held, released;
		Mouse mouse;
		int keys[348];
		int pad[8][14];

		float deltaTime;
		float time;
		float renderTime;
		bool pressed;
		bool hold;
		bool showFPS;
		bool maximized = false;
		bool displayUI = false;
		bool playToggled = false;
		bool playMode = false;
	};

#endif // !INPUT_H
}