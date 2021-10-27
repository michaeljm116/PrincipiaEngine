/* Copyright (C) Mike Murrell 2017 Window class
a singleton class that handles the window
probably should be a system instead */

#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

//#define UI_ENABLED

namespace Principia {
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
		int				getHeight() { return height; }
		int				getWidth() { return width; }


		Window(Window const&) = delete;
		void operator=(Window const&) = delete;
		~Window();



	private:
		GLFWwindow* window;
		GLFWmonitor* primary;
		int			height;
		int			width;
		bool		maximized;
		const GLFWvidmode* mode;
	};

}
#endif // !WINDOW_H
