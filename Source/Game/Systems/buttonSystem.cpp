#include "buttonSystem.h"

ButtonSystem::ButtonSystem()
{
	addComponentType<InputComponent>();
	addComponentType<ButtonComponent>();
	addComponentType<NodeComponent>();
	addComponentType<MaterialComponent>();
}

ButtonSystem::~ButtonSystem()
{
}

void ButtonSystem::initialize()
{
	em = world->getEntityManager();
	sm = world->getSystemManager();
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();
	ts = (TransformSystem*)sm->getSystem<TransformSystem>();

	inputMapper.init(*world);
	buttonMapper.init(*world);
	nodeMapper.init(*world);
	materialMapper.init(*world);
}

void ButtonSystem::processEntity(artemis::Entity & e)
{
	ButtonComponent* bc = buttonMapper.get(e);
	InputComponent* ic = inputMapper.get(e);
	NodeComponent* nc = nodeMapper.get(e);
	MaterialComponent* mc = materialMapper.get(e);

	if (bc->collided) {
		bc->collided = !bc->collided;
		if (ic->direction.y > 0.99f) { 
			bc->pressed = !bc->pressed;
			//OPen door now
			RigidBodyComponent* rbc = (RigidBodyComponent*)nc->children[0]->data->getComponent<RigidBodyComponent>();
			MaterialComponent* dmc = (MaterialComponent*)nc->children[0]->data->getComponent<MaterialComponent>();
			rbc->toggled = true;
			//tit = 4 black = 8 red = 12 green = 11
			bc->pressed ? dmc->matID = 4 : dmc->matID = 8;
			bc->pressed ? mc->matID = 12 : mc->matID = 11;

			//rs->updateMaterial(mc->matID);
			//rs->updateMaterial(dmc->matID);

			PrimitiveComponent* oc = (PrimitiveComponent*)nc->data->getComponent<PrimitiveComponent>();
			PrimitiveComponent* doc = (PrimitiveComponent*)nc->children[0]->data->getComponent<PrimitiveComponent>();

			rs->getObject(oc->objIndex).matId = mc->matID;
			rs->getObject(doc->objIndex).matId = dmc->matID;

			rs->setRenderUpdate(RenderSystem::UPDATE_OBJECT);
		}
	}
}
