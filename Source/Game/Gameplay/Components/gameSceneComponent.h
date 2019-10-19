#pragma once
#include <Artemis/Component.h>

struct GameSceneComponent : artemis::Component {
	int levelIndex = 0;
	GameSceneComponent(const int& i) { levelIndex = i; };
};