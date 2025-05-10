#pragma once
/* Copyright (C) Mike Murrell 2018 Key class
*/
#include <string>

namespace Principia {
	enum KeyStates {
		KEY_NONE = 0x01,
		KEY_PRESSED = 0x02,
		KEY_HELD = 0x04,
		KEY_RELEASED = 0x08,
	};



	struct Key {
		unsigned char state = KEY_NONE;
		int key = 0;
		std::string name = "";

		Key() {};
		Key(int key, std::string name) : key(key), name(name) {};
		void update(bool pressed) {
			if (pressed) {
				if (state & KEY_NONE)
					state = KEY_HELD | KEY_PRESSED;
				else if (state & KEY_PRESSED)
					state = KEY_HELD;
			}
			else
			{
				if (state & KEY_PRESSED | KEY_HELD)
					state = KEY_RELEASED | KEY_NONE;
				else if (state & KEY_RELEASED | KEY_NONE)
					state = KEY_NONE;
			}
		};
	};
}