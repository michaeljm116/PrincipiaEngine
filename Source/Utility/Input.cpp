#include "../pch.h"
#include "Input.h"
#include "window.h"
#include <chrono>
#include "../imgui/imgui.h"

//#define UIIZON
namespace Principia {

#pragma region Input
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
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
#pragma endregion Callbacks

	Input::~Input()
	{
	}

	void Input::init() {

		//set up callbacks
		window = Window::get().getWindow();
		glfwSetKeyCallback(window, key_callback);
		glfwSetCharCallback(window, char_callback);
		glfwSetCursorPosCallback(window, cursor_position_callback);
		//glfwSetMouseButtonCallback(window, Input::mouse_button_callback);
		glfwSetInputMode(window, GLFW_STICKY_KEYS, true);
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetJoystickCallback(joystick_callback);
		
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