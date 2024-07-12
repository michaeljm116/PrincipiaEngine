#pragma once
#include <Artemis/Component.h>
#include <string>

namespace Principia {
	struct Cmp_Debug : artemis::Component
	{
		std::string type = "";
		std::string message = "";
	};
}