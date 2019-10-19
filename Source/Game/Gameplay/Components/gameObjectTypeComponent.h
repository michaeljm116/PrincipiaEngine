#pragma once
#include "Artemis/Component.h"

namespace Principia {
	enum GameObjectType {
		GAMEOBJECT_NONE = 0x00,
		GAMEOBJECT_PLAYER = 0x01,
		GAMEOBJECT_ENEMY = 0x02,
		GAMEOBJECT_PROJECTILE = 0x04,
		GAMEOBJECT_ENVIRONMENT = 0x10
	};

	struct GameObjectTypeComponent : public artemis::Component {
		GameObjectType type;
		GameObjectTypeComponent(GameObjectType t) : type(t) {};
	};
}