#include <Artemis\Artemis.h>
#include "../Utility/resourceManager.h"
#include "../Game/script.hpp"
#include "../Game/transformSystem.h"

class Scene : public artemis::EntityProcessingSystem {
public: 
	Scene() { script.loadScript("../Game/testscript.xml"); };// artemis::World w) : world(w) {};
	~Scene() {};


	//artemis::World world;
	artemis::EntityManager* em;
	artemis::SystemManager* sm;
	RenderSystem* rs;
	TransformSystem* ts;
	EngineUISystem* ui;
	
	artemis::Entity* barrel;
	artemis::Entity* houseTower;
	artemis::Entity* wheel;
	artemis::Entity* sphere;
	artemis::Entity* box;
	artemis::Entity* diamond;
	artemis::Entity* light;
	artemis::Entity* camera;

	Scripto script;

	//std::vector<artemis::Entity*> meshEntities;

	void init() {
		em = world->getEntityManager();
		sm = world->getSystemManager();
		rs = (RenderSystem*)sm->getSystem<RenderSystem>();
		ts = (TransformSystem*)sm->getSystem<TransformSystem>();
		ui = (EngineUISystem*)sm->getSystem<EngineUISystem>();

		camera = createCamera();
		light = createLight();
		barrel = createMesh("Barrel", script.vData[4], script.vData[5], script.vData[3]);
		houseTower = createMesh("HouseTower", script.vData[1], script.vData[2], script.vData[0]);
		sphere = createSphere(glm::vec3(1.75f, -0.75f, 0.5f), 1.f, 7);
		box = createBox(glm::vec3(-1.75f, 2.75f, -2.25f), glm::vec3(1.f), 8);
	};
	
	void doStuff() {
		ts->Scale(*barrel);
		ts->Translate(*barrel);
		
		ts->Scale(*houseTower);
		ts->Translate(*houseTower);
		rs->updateMesh();

		ts->ScaleX(*sphere, ObjectType::Sphere);
		ts->ScaleX(*box, ObjectType::Box);
		ts->TranslateX(*sphere, ObjectType::Sphere);
		ts->TranslateX(*box, ObjectType::Box);

		rs->updateBox();
		rs->updateSphere();
		/*
		NodeComponent* parent = (NodeComponent*)barrel->getComponent<NodeComponent>();
		for each (NodeComponent* child in parent->children)
		{
			MaterialComponent* mat = (MaterialComponent*)child->data->getComponent<MaterialComponent>();
			mat.
		}*/
	}
	artemis::Entity* createMesh(std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 sca) {
		//Create Entity
		//ent = *em->create();
		//entity = &em->create();

		//Add Mesh Component and make it a parent node
		artemis::Entity* entity = &em->create();
		rMesh* mesh = &RESOURCEMANAGER.getMesh(name);
		entity->addComponent(new MeshComponent(RESOURCEMANAGER.getMeshIndex(name)));
		entity->addComponent(new AABBComponent());	//MeshAABB's point to the physics system
		entity->addComponent(new NodeComponent(entity));
		entity->addComponent(new TransformComponent(pos, rot, sca));
		NodeComponent* parent = (NodeComponent*)entity->getComponent<NodeComponent>();
		parent->name = name;

		//set up the subsets
		int i = 0;
		for (std::vector<rSubset>::const_iterator itr = mesh->subSets.begin(); itr != mesh->subSets.end(); itr++) {

			//Create Entity
			artemis::Entity* child = &em->create();

			//Set up subset data
			child->addComponent(new SubsetComponent());
			child->addComponent(new MaterialComponent());
			child->addComponent(new AABBComponent());	//SubsetAABB's point to the rendering system
			child->addComponent(new NodeComponent(child, parent));
			NodeComponent* childNode = (NodeComponent*)child->getComponent<NodeComponent>();

			++i;
			childNode->name = "Child " + std::to_string(i);
			parent->children.push_back(childNode);
		}

		rs->addMesh(*entity);
		ui->addParentEntity(entity, name);
		return entity;
	}
	/*void createMesh(artemis::Entity* entity, std::string name) {
		createMesh(entity, name, glm::vec3(1.f), glm::vec3(1.f), glm::vec3(1.f));
	}*/

	artemis::Entity* createSphere(glm::vec3 pos, float radius, int matID) {
		//So im guessing its like... first create entity
		//then in the render system, create a rendery sphere
		//also have material stuff
		//then in the physics system, create a physicy sphere
		//the data of the box will be in the renderer
		//scene will point to the render data
		//physics will point to the render data
		artemis::Entity* e = &em->create();
		ssSphere sphere;
		sphere.pos = pos;
		sphere.radius = radius;
		sphere.matID = matID; //maybe?!??

		e->addComponent(new SphereComponent());
		e->addComponent(new MaterialComponent());
		e->addComponent(new NodeComponent(e));
		e->addComponent(new TransformComponent(pos, glm::vec3(0.f), glm::vec3(radius)));

		//TransformComponent* tc = (TransformComponent*)e->getComponent<TransformComponent>();
		//tc->position = pos;
		//tc->scale = glm::vec3(radius);
		
		rs->addSphere(*e, sphere);
		ui->addParentEntity(e, "Sphere " + rs->getSphereSize());
		NodeComponent* parent = (NodeComponent*)e->getComponent<NodeComponent>();
		parent->name = "New Sphere " + rs->getSphereSize();
		return e;
	}

	artemis::Entity* createBox(glm::vec3 center, glm::vec3 extents, int matID) {
		artemis::Entity* e = &em->create();
		ssBox box;
		box.center = center;
		box.extents = extents;
		box.matID = matID;

		e->addComponent(new BoxComponent());
		e->addComponent(new MaterialComponent());
		e->addComponent(new NodeComponent(e));
		e->addComponent(new TransformComponent(center, glm::vec3(0.f), extents));

		//TransformComponent* tc = (TransformComponent*)e->getComponent<TransformComponent>();
		//tc->position = center;
		//tc->scale = extents;
		//tc->rotation = idkmybffjill;

		rs->addBox(*e, box);
		ui->addParentEntity(e, "Box " + rs->getBoxSize());
		NodeComponent* parent = (NodeComponent*)e->getComponent<NodeComponent>();
		parent->name = "New Box " + rs->getBoxSize();
		return e;

	}

	artemis::Entity* createLight(){//glm::vec3 pos, glm::vec3 color, float intensity) {
		artemis::Entity* e = &em->create();

		e->addComponent(new LightComponent());
		//LightComponent* l = new LightComponent();
		//l->color = color;
		//l->intensity = intensity;

		e->addComponent(new TransformComponent(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f)));
		e->addComponent(new NodeComponent(e));
		ui->addParentEntity(e, "Light");
		NodeComponent* parent = (NodeComponent*)e->getComponent<NodeComponent>();
		parent->name = "Light";
		rs->addLight(*e);

		return e;

	}

	artemis::Entity* createCamera(){// glm::vec3 pos) {
		artemis::Entity* e = &em->create();

		e->addComponent(new CameraComponent());
		e->addComponent(new TransformComponent(glm::vec3(0), glm::vec3(0.f), glm::vec3(0.f)));
		e->addComponent(new NodeComponent(e));
		ui->addParentEntity(e, "Camera");
		NodeComponent* parent = (NodeComponent*)e->getComponent<NodeComponent>();
		parent->name = "Camera";
		rs->addCamera(*e);

		return e;
	}

	void processEntity(artemis::Entity &e) {};
};