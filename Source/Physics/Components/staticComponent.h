#pragma once
#include <Artemis/Component.h>
namespace Principia {
	struct StaticComponent : public artemis::Component {
		bool isStatic = true;
	};
}