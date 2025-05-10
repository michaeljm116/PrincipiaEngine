#pragma once
#include <Artemis/EntityProcessingSystem.h>
namespace Principia {

	struct ScriptComponent;

	class Sys_Scripting : public artemis::EntityProcessingSystem {
	private:
		artemis::ComponentMapper<ScriptComponent> scriptMapper = {};

	public:
		Sys_Scripting();
		~Sys_Scripting();

		void initialize() override;
		void added(artemis::Entity& e) override;
		void removed(artemis::Entity& e) override;
		void processEntity(artemis::Entity& e) override;


	};
}