/* Copyright (C) Mike Murrell 2017 Window class
a singleton class that handles the window
probably should be a system instead
*/

#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#ifndef  GLFW_INCLUDE_VULKAN
#define GLFW_INCLUE_VULKAN
#endif

#include <GLFW/glfw3.h>

//#define UI_ENABLED

#define WINDOW Window::get()

class Window
{
public:
	static Window& get() {
		static Window instance;

		return instance;
	}
private: 
	Window() {}

public: 
	void init();
	void update();
	void toggleMaximized();
	void resize();


	GLFWwindow*		getWindow() { return window; }
	uint16_t		getHeight() { return height; }
	uint16_t		getWidth()	{ return width; }


	Window(Window const&) = delete;
	void operator=(Window const&) = delete;
	~Window();

	

private:
	GLFWwindow* window;
	GLFWmonitor* primary;
	uint16_t	height;
	uint16_t	width;
	bool		maximized;
	const GLFWvidmode* mode;
};

#endif // !WINDOW_H
