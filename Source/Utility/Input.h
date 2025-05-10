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

#include "../Utility/key.h"
#include "../Utility/mouse.h"

struct GLFWwindow;

namespace Principia {

#define pINPUT Input::get()

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

		GLFWwindow* window = nullptr;

		void KeyDirection(int key, bool pressed);




	public:
		void init();
		void update();

		//bool up, down, left, right;
		//bool pressed, held, released;
		Mouse mouse = Mouse();
		int keys[348] = {};
		int pad[8][14] = {};

		float deltaTime = 0.f;
		float time = 0.f;
		float renderTime = 0.f;
		bool pressed = false;
		bool hold = false;
		bool showFPS = false;
		bool maximized = false;
		bool displayUI = false;
		bool playToggled = false;
		bool playMode = false;

		bool hasGamepad = false;
	};

#endif // !INPUT_H
}