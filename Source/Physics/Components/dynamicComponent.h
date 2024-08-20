#pragma once
#include <Artemis/Component.h>
namespace Principia {
	struct DynamicComponent : public artemis::Component {
		bool isDynamic = true;
	};
}