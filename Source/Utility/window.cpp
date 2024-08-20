#include "../pch.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>

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
		window = glfwCreateWindow(width, height, "RayTracy Bird", nullptr, nullptr);//nullptr , nullptr);


		GLFWimage icons[1];
		std::string path = "../Assets/Levels/Test/Textures/window_icon.png";
		icons[0].pixels = stbi_load(path.c_str(), &icons->width, &icons->height, 0, STBI_rgb_alpha);
		glfwSetWindowIcon(window, 1, icons);
		stbi_image_free(icons[0].pixels);

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