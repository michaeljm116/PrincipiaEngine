#pragma once
#include "Artemis/Component.h"

/*
-1 1   0 1   1 1

-1 0   0 0   1 0

-1-1   0-1   1-1

02 12 22
01 11 21
00 10 20

0010 0110 1010
0001 0101 1001
0000 0100 1000

2 6 10
1 5 9
0 4 8

default = 5,
up = 6,
upleft = 2,
left = 1,
downleft = 0,
down = 4,
downright = 8,
right = 9,
upright = 10

*/
enum class RotationDir {
	downleft,
	left,
	upleft,
	null1,
	down,
	default,
	up,
	null2,
	downright,
	right,
	upright
};

struct CharacterRotationComponent : public artemis::Component {
	RotationDir dir;
	float movementMul = 1;
	CharacterRotationComponent(RotationDir d) : dir(d) {};
	RotationDir convert(int x, int y) {
		x++; y++;
		int res = y;
		res |= x << 2;
		return RotationDir(res);
	}
};