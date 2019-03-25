#include "window.h"

/*
Window::Window()
{
height = 720;
width = 1280;
}*/
Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::init() {
	height = 720;
	width = 1280;
	maximized = false;


	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	primary = glfwGetPrimaryMonitor();
	mode = glfwGetVideoMode(primary);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	//glfwGetPrimaryMonitor() , nullptr);
	window = glfwCreateWindow(width, height, "Vulkany", nullptr, nullptr);//nullptr , nullptr);
}

void Window::update() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void Window::toggleMaximized()
{
	if (maximized) {
		maximized = false;
		glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else {
		maximized = true;
		glfwMaximizeWindow(window);
	}
}

void Window::resize() {
	int _width, _height;
	glfwGetWindowSize(window, &_width, &_height);
	width = _width;
	height = _height;
	
}


