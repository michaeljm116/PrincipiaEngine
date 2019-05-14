#pragma once
#include <Artemis/Artemis.h>

enum class AppState
{
	Startup, TitleScreen, MainMenu, Options, CharacterSelect,
	Play, Paused, Editor
};

struct ApplicationComponent : public artemis::Component
{
	AppState state = AppState::TitleScreen;
	bool transition = false;
};

struct TitleComponent : public artemis::Component {
	float timer = 2.f;
	TitleComponent() {};
};

struct MenuComponent : public artemis::Component {
	
	MenuComponent() {};
};

struct PauseComponent : public artemis::Component {

	PauseComponent() {};
};

struct EditorComponent : public artemis::Component {

	EditorComponent() {};
};
enum class GameMode {
	Traditional, XZ
};


struct GameComponent : artemis::Component {
	GameMode mode;
	GameComponent(GameMode m) : mode(m) {};// { mode = GameMode::Traditional; };
	GameComponent() { mode = GameMode::Traditional; };
};
