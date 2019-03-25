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

	//set up keys
	Keys[0].key = GLFW_KEY_W;
	Keys[0].name = "Fwd";
	Keys[1].key = GLFW_KEY_S;
	Keys[1].name = "Bwd";
	Keys[2].key = GLFW_KEY_A; 
	Keys[2].name = "Right";
	Keys[3].key = GLFW_KEY_D;
	Keys[3].name = "Left";
	Keys[4].key = GLFW_KEY_LEFT_ALT;
	Keys[4].name = "Down";
	Keys[5].key = GLFW_KEY_SPACE;
	Keys[5].name = "Up";
	Keys[6].key = GLFW_KEY_P;
	Keys[6].name = "Play";

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

	for (int i = 0; i < 6; ++i) {
		if (key == Keys[i].key) { 
			Keys[i].update(pressed);
			return;
		}
	}
}

void Input::update()
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float newTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
	deltaTime = time - newTime;
	time = newTime;
}

void Input::updateAxis(int key, bool pressed) {
	
	KeyDirection(key, pressed);

	glm::vec3 tempA = glm::vec3(0);
	if (Keys[0].state & KEY_HELD) tempA.z -= 1;
	if (Keys[1].state & KEY_HELD) tempA.z += 1;
	if (Keys[2].state & KEY_HELD) tempA.x += 1;
	if (Keys[3].state & KEY_HELD) tempA.x -= 1;
	if (Keys[4].state & KEY_HELD) tempA.y += 1;
	if (Keys[5].state & KEY_HELD) tempA.y -= 1;

	axis = tempA;
}
