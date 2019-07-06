#include "pch.h"
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
#include "Game/Application/applicationSystem.h"

int main() {

	artemis::World world;
	artemis::SystemManager * sm = world.getSystemManager();
	artemis::TagManager * tm = world.getTagManager();
	

	RenderSystem*	 renderSys	  = (RenderSystem*)	  sm->setSystem(new RenderSystem());
	EngineUISystem*	 engineUISys  = (EngineUISystem*) sm->setSystem(new EngineUISystem());
	TransformSystem* transformSys = (TransformSystem*)sm->setSystem(new TransformSystem());
	PhysicsSystem*	 physicsSys	  = (PhysicsSystem*)  sm->setSystem(new PhysicsSystem());
	AnimationSystem* animSys	  = (AnimationSystem*)sm->setSystem(new AnimationSystem());
	ApplicationSystem* appSys	  = (ApplicationSystem*)sm->setSystem(new ApplicationSystem());

	artemis::EntityManager * em = world.getEntityManager();

	static artemis::Entity* singletonEntity = &em->create();
	world.setSingleton(singletonEntity);

	Resources::get().LoadConfig("");
	Resources::get().LoadMaterials("../Assets/Levels/Pong/Materials.xml");
	Resources::get().LoadDirectory("../Assets/Levels/Pong/Models/");
	Resources::get().LoadAnimations("../Assets/Levels/Pong/Animations/");
	//Resources::get().LoadMaterials("../Assets/Levels/Level1/Materials.xml");
	//Resources::get().LoadDirectory("../Assets/Levels/Level1/Models/");
	//Resources::get().LoadAnimations("../Assets/Levels/Level1/Animations/");



	singletonEntity->addComponent(new ApplicationComponent());
	singletonEntity->addComponent(new GUIComponent(glm::vec2(0.0f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f), 0, 1, false));
	singletonEntity->addComponent(new EditorComponent);
	//singletonEntity->addComponent(new TitleComponent());
	GlobalController* controller = new GlobalController();
	for (int i = 0; i < NUM_GLOBAL_BUTTONS; ++i)
		controller->buttons[i].key = Resources::get().getConfig().controllerConfigs[0][i];
	singletonEntity->addComponent(controller);

	singletonEntity->refresh();
	appSys->change(*singletonEntity);

	////////////////////COMPONENTTYPETEST///////////////////////////////
	//auto bob = em->getComponents(*singletonEntity);
	//int count = bob.getCount();
	//for (int i = 0; i < count; ++i) {
	//	std::cout << typeid(bob.get(i)).name() << std::endl;
	//	artemis::ComponentType t;
	//	t = artemis::ComponentTypeManager::getTypeFor(typeid(bob.get(i)));
	//	t.getId();
	//}
	//auto ac = (ApplicationComponent*)singletonEntity->getComponent<ApplicationComponent>();
	

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
		appSys->initialize();
		//appSys->instantGameStart();

		static std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
		static std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
		static std::chrono::duration<float> duration;

		
		while (!glfwWindowShouldClose(WINDOW.getWindow())) {
			duration = end - start;
			world.loopStart();
			float delta = duration.count();
			if (delta > 0 && delta < 1.f)
				world.setDelta(delta);
			else
				world.setDelta(0.001f);
			glfwPollEvents();
			start = std::chrono::high_resolution_clock::now();
			INPUT.update();

			appSys->process();

			end = std::chrono::high_resolution_clock::now();
		}

		vkDeviceWaitIdle(renderSys->vkDevice.logicalDevice); //so it can destroy properly
	//	world.~World();
		renderSys->cleanup();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}

