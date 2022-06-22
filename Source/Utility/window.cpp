#include "../pch.h"
#include "window.h"

/*
Window::Window()
{
height = 720;
width = 1280;
}*/

namespace Principia {
	Window::~Window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::init() {
		height = 720;
		width = 1280; //1280;//1680;// 1280;
#ifdef UIIZON
		width = 1280;
#endif // UIIZON

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
		resize();
	}

	void Window::resize() {
		glfwGetWindowSize(window, &width, &height);

	}
}