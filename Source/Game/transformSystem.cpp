#include "transformSystem.h"

/*
So when you translate something, it's either going to be a...
camera
light
mesh
plane
sphere
box
particle?


and there's also the question of...
should the translation be separate
the way things work is, you translate using components
and the system does thing automatically
*/
TransformSystem::TransformSystem()
{
	addComponentType<RigidBodyComponent>();
	addComponentType<NodeComponent>();
}

TransformSystem::~TransformSystem()
{
}

void TransformSystem::initialize()
{
	transformMapper.init(*world);
	nodeMapper.init(*world);

	em = world->getEntityManager();
	sm = world->getSystemManager();
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();

}

void TransformSystem::processEntity(artemis::Entity & e)
{
	TransformComponent* tc = transformMapper.get(e);
	NodeComponent* nc = nodeMapper.get(e);

	//tc->global.rotation = glm::toQuat(tc->world);
	//tc->global.position = tc->world[3];
	//tc->global.scale = tc->local.scale;
	
	size_t numChildren = nc->children.size();
	for (int c = 0; c < numChildren; c++) {
		SQTTransform(nc, tc->local);
	}
}

void TransformSystem::SQTTransform(NodeComponent * nc, sqt parent)
{
	//Transform errthang...
	TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();

	tc->global.position = parent.position + tc->local.position;
	tc->global.rotation = parent.rotation * tc->local.rotation;
	tc->global.scale = parent.scale * tc->local.scale;

	tc->world = glm::toMat4(tc->global.rotation);
	tc->world[3] = glm::vec4(tc->global.position, 1.f);

	//pass in the transform info as well as the components to mult
	if (nc->flags & COMPONENT_MODEL)
	{
	}
	else if (nc->flags & COMPONENT_PRIMITIVE) {
		
		PrimitiveComponent* objComp = (PrimitiveComponent*)nc->data->getComponent<PrimitiveComponent>();
		ssPrimitive& obj = rs->getObject(objComp->objIndex);
		//so what im looking for is the resource manager's

		//scale the aabb
		obj.center = tc->global.position;
		obj.extents = tc->global.scale;// rotateAABB(tc->global.rotation, obj.extents * tc->global.scale);
		obj.world = tc->world;

		rs->setRenderUpdate(RenderSystem::UPDATE_OBJECT);
	}
	else if (nc->flags & COMPONENT_CAMERA) {
		CameraComponent* c = (CameraComponent*)nc->data->getComponent<CameraComponent>();
		c->pos = tc->global.position;// nodeTrans->position;
		rs->updateCamera(c);
	}
	else if (nc->flags & COMPONENT_LIGHT) {
		LightComponent* l = (LightComponent*)nc->data->getComponent<LightComponent>();
		ssLight& light = rs->getLight(l->id);
		light.color = l->color;
		light.intensity = l->intensity;
		light.pos = tc->global.position;

		rs->setRenderUpdate(RenderSystem::UPDATE_LIGHT);
	}
	if (nc->children.size() > 0) {
		for each (NodeComponent* child in nc->children)
		{
			SQTTransform(child, tc->global);// &tc, rotM, transM, scaleM, false);
		}
	}
}

void TransformSystem::recursiveTransform(NodeComponent* nc){//, TransformComponent global) {
	bool hasParent = nc->parent == nullptr ? false : true;
	//geometryTransformConverter(nc);
	TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();

	//set up all the matrices
	//if (!hasParent) {		glm::mat4 rotationM;
	glm::mat4 rotationM;
	rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	tc->local.rotation = glm::toQuat(rotationM);

	tc->global = tc->local;
	if (hasParent) {
		TransformComponent* ptc = (TransformComponent*)nc->parent->data->getComponent<TransformComponent>();
		tc->global.position = ptc->global.position + tc->local.position;
		tc->global.rotation =  tc->local.rotation * ptc->global.rotation;
		tc->global.scale = ptc->global.scale * tc->local.scale;
	}

	tc->world = glm::toMat4(tc->global.rotation);
	tc->world[3] = glm::vec4(tc->global.position, 1.f);
	if (nc->flags & COMPONENT_MODEL)
	{ }
	else if (nc->flags & COMPONENT_PRIMITIVE) {
		//GET THE OBJ
		PrimitiveComponent* objComp = (PrimitiveComponent*)nc->data->getComponent<PrimitiveComponent>();
		ssPrimitive& obj = rs->getObject(objComp->objIndex);

		//scale the aabb
		obj.center = tc->world[3];// tc->global.position;
		obj.extents = tc->global.scale * 1.f;// rotateAABB(tc->global.rotation, tc->global.scale);
		obj.world = tc->world;// *glm::vec4(tc->global.scale, 1.f);

		rs->setRenderUpdate(RenderSystem::UPDATE_OBJECT);
	}

	else if (nc->flags & COMPONENT_CAMERA) {
		CameraComponent* c = (CameraComponent*)nc->data->getComponent<CameraComponent>();
		c->pos = tc->global.position;// nodeTrans->position;
		rs->updateCamera(c);
	}
	else if (nc->flags & COMPONENT_LIGHT) {
		LightComponent* l = (LightComponent*)nc->data->getComponent<LightComponent>();
		ssLight& light = rs->getLight(l->id);
		light.color = l->color;
		light.intensity = l->intensity;
		light.pos = tc->global.position;

		rs->setRenderUpdate(RenderSystem::UPDATE_LIGHT);
	}

	if (nc->children.size() > 0) {
		for each (NodeComponent* child in nc->children)
		{
			recursiveTransform(child);// &tc, rotM, transM, scaleM, false);
		}
	}
}

glm::vec3 TransformSystem::rotateAABB(const glm::quat & m, const glm::vec3 & extents)
{
	//set up cube
	glm::vec3 v[8];
	v[0] = extents;
	v[1] = glm::vec3(extents.x, extents.y, -extents.z);
	v[2] = glm::vec3(extents.x, -extents.y, -extents.z);
	v[3] = glm::vec3(extents.x, -extents.y, extents.z);
	v[4] = glm::vec3(-extents);
	v[5] = glm::vec3(-extents.x, -extents.y, extents.z);
	v[6] = glm::vec3(-extents.x, extents.y, -extents.z);
	v[7] = glm::vec3(-extents.x, extents.y, extents.z);

	//transform them
#pragma omp parallel for
	for (int i = 0; i < 8; ++i) {
		v[i] = abs(m * v[i]);// glm::vec4(v[i], 1.f));
		
	}

	//compare them
	glm::vec3 vmax = glm::vec3(FLT_MIN);
	for (int i = 0; i < 8; ++i) {
		vmax.x = tulip::max(vmax.x, v[i].x);
		vmax.y = tulip::max(vmax.y, v[i].y);
		vmax.z = tulip::max(vmax.z, v[i].z);
	}

	return vmax;
}

void TransformSystem::geometryTransformConverter(NodeComponent * nc)
{
	if (nc->flags & COMPONENT_MESH | COMPONENT_MODEL | COMPONENT_BOX)
		return;
	TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();
	if (nc->flags & COMPONENT_SPHERE) {
		tc->global.scale.y = tc->global.scale.x;
		tc->global.scale.z = tc->global.scale.x;
	}
	if (nc->flags & COMPONENT_CYLINDER) {
		tc->global.scale.z = tc->global.scale.x;
	}
}
