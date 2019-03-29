
#include "Utility/window.h"
#include "Utility/Input.h"
#include "Rendering/rendermanagers.h"
#include "Rendering/renderSystem.h"
#include "Rendering/engineUISystem.h"
#include "Physics/physicsSystem.h"
#include "ArtemisFrameWork/Artemis/Artemis.h"
#include "Utility/resourceManager.h"
#include "Game/scene.h"
#include "Game/transformSystem.h"
#include "Game/gameSystem.h"

int main() {

	artemis::World world;
	artemis::SystemManager * sm = world.getSystemManager();

	RenderSystem*	 renderSys	  = (RenderSystem*)	  sm->setSystem(new RenderSystem());
	EngineUISystem*	 engineUISys  = (EngineUISystem*) sm->setSystem(new EngineUISystem());
	TransformSystem* transformSys = (TransformSystem*)sm->setSystem(new TransformSystem());
	PhysicsSystem*	 physicsSys	  = (PhysicsSystem*)  sm->setSystem(new PhysicsSystem());
	AnimationSystem* animSys	  = (AnimationSystem*)sm->setSystem(new AnimationSystem());

	artemis::EntityManager * em = world.getEntityManager();
	Resources::get().LoadMaterials("../Assets/Levels/Level1/Materials.xml");
	Resources::get().LoadAnimations("../Assets/Levels/Level1/Animations/");
	Resources::get().LoadDirectory("../Assets/Levels/Level1/Models/");

	try {
		Window::get().init();
		Input::get().init();
		transformSys->initialize();
		physicsSys->initialize();
		//physicsSys->addGroundForNow();
		renderSys->preInit();
		Scene::get().init(world);
		Scene::get().doStuff();
		renderSys->initialize();
		animSys->initialize();

		GAMESYSTEM.init(world);

		renderSys->buildBVH();

		while (!glfwWindowShouldClose(WINDOW.getWindow())) {

			glfwPollEvents();
			INPUT.update();
			if (INPUT.playToggled) {
				INPUT.playToggled = !INPUT.playToggled;
				renderSys->togglePlayMode();
				physicsSys->togglePlayMode();
			}
			if(INPUT.playMode){
				GAMESYSTEM.update(INPUT.deltaTime);
				physicsSys->update();
				physicsSys->process(); 
				GAMESYSTEM.updateCamera();
			
			}
			animSys->update(INPUT.deltaTime);
			animSys->process();
			renderSys->mainLoop();
		}

		vkDeviceWaitIdle(renderSys->vkDevice.logicalDevice); //so it can destroy properly
		renderSys->cleanup();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}

