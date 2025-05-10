#pragma once
#include <Artemis/Component.h>
namespace Principia {
	struct CmpGravity : public artemis::Component {
		bool grounded = false;
		float gforce = 19.807f;
		float diff = 0;
	};
}