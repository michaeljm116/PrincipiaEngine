#include "Input.h"
#include "window.h"
Input::~Input()
{
}

void Input::init() {
	//set up callbacks
	window = Window::get().getWindow();
	glfwSetKeyCallback(window, Input::key_callback);
	glfwSetCharCallback(window, Input::char_callback);
	glfwSetCursorPosCallback(window, Input::cursor_position_callback);
	glfwSetMouseButtonCallback(window, Input::mouse_button_callback);
	glfwSetScrollCallback(window, Input::scroll_callback);

	/*
	Q, E, R, F, V
	space, tab, shift
	1, 2, 3, 4, 5

	awdsdqdeqdewfvafadfvaddawda         
	
	*/

	//set up mouse
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	mouse.x = x;
	mouse.prevX = x;
	mouse.y = y;
	mouse.prevY = y;

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
}
