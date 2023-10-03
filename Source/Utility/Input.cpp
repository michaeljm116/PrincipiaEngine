#include "Input.h"
#include "window.h"
#include <chrono>
namespace Principia {
	Input::~Input()
	{
	}

	void Input::init() {

		//set up callbacks
		window = Window::get().getWindow();
		glfwSetKeyCallback(window, Input::key_callback);
		glfwSetCharCallback(window, Input::char_callback);
		glfwSetCursorPosCallback(window, Input::cursor_position_callback);
		//glfwSetMouseButtonCallback(window, Input::mouse_button_callback);
		glfwSetInputMode(window, GLFW_STICKY_KEYS, true);
		glfwSetScrollCallback(window, Input::scroll_callback);
		glfwSetJoystickCallback(Input::joystick_callback);
		
		for (int i = 0; i < 16; ++i) {
			if (glfwJoystickPresent(i)) {
				int axesCount;
				const float* axes = glfwGetJoystickAxes(i, &axesCount);

				int buttonCount;
				const unsigned char* buttons = glfwGetJoystickButtons(i, &buttonCount);

				const char* name = glfwGetJoystickName(i);
				const char* game_pad_name = "NOT a Gamepad";
				bool is_game_pad = false;
				GLFWgamepadstate state;
				if (glfwJoystickIsGamepad(i)) {
					is_game_pad = true;
					hasGamepad = true;
					game_pad_name = glfwGetGamepadName(i);
					glfwGetGamepadState(i, &state);
				}
				//printf("%s: \t Axes:%f \t buttons:%c", name, axes, buttons);
				printf("\n%u is a %s aka %s it has %u Axes and %u buttons\n", i, name, game_pad_name, axesCount, buttonCount);
			}
		}




		/*
		Q, E, R, F, V
		space, tab, shift
		1, 2, 3, 4, 5

		awdsdqdeqdewfvafadfvaddawda

		*/

		//set up mouse

		double x, y;
		glfwGetCursorPos(window, &x, &y);
		mouse.x = static_cast<int>(x);
		mouse.prevX = static_cast<int>(x);
		mouse.y = static_cast<int>(y);
		mouse.prevY = static_cast<int>(y);

		//set up timer
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
		deltaTime = time;
		showFPS = true;
	}

	void Input::KeyDirection(int key, bool pressed) {

		//for (int i = 0; i < 6; ++i) {
		//	if (key == Keys[i].key) { 
		//		Keys[i].update(pressed);
		//		return;
		//	}
		//}
	}

	void Input::update()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float newTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
		deltaTime = abs(time - newTime);
		time = newTime;

		int left_mb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		int right_mb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
		mouse.updateButton(GLFW_MOUSE_BUTTON_LEFT, left_mb);
		mouse.updateButton(GLFW_MOUSE_BUTTON_RIGHT, right_mb);
		
	}

}