#pragma once
#include <Artemis/Artemis.h>
#include <glm/glm.hpp>

enum class GameState
{
	Startup, TitleScreen, MainMenu, Options, CharacterSelect,
	Play, Paused, Editor
};

enum class GameMode {
	Traditional, XZ
};


struct GameComponent : artemis::Component {
	GameState state;
	GameMode mode;
	GameComponent(GameState s) : state(s) { mode = GameMode::Traditional; };
};

