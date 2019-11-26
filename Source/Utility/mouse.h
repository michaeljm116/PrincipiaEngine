#pragma once
/* Copyright (C) Mike Murrell 2018 Mouse class
*/

namespace Principia {
	enum MouseStates {
		MOUSE_NONE = 0x01,
		MOUSE_PRESSED = 0x02,
		MOUSE_HELD = 0x04,
		MOUSE_RELEASED = 0x08
	};


	struct Mouse {
		int x;
		int y;
		int prevX;
		int prevY;

		int buttons[12];

		double scroll;
		bool   active;

		Mouse() {
			x = 0;
			y = 0;
			prevX = 0;
			prevY = 0;

			scroll = 0;
			active = true;

			for (int i = 0; i < 12; ++i) {
				buttons[i] = 0;
			}
		}

		/*void updateButton(int btn, bool pressed) {
			if (active) {
				if (pressed) {
					if (button[btn] & MOUSE_NONE)
						button[btn] = MOUSE_HELD | MOUSE_PRESSED;
					else if (button[btn] & MOUSE_PRESSED)
						button[btn] = MOUSE_HELD;
				}
				else { //not pressed
					if (button[btn] & MOUSE_PRESSED | MOUSE_HELD)
						button[btn] = MOUSE_RELEASED | MOUSE_NONE;
					else if (button[btn] & MOUSE_RELEASED | MOUSE_NONE)
						button[btn] = MOUSE_NONE;
				}
			}
		};*/

		void updatePosition(int _x, int _y) {
			if (active) {
				prevX = x;
				prevY = y;
				x = _x;
				y = _y;
			}
		}

		void updateScroll(double offset) {
			if (active)
				scroll = offset;
		}

	};
}