//#include "../pch.h"
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

namespace Principia {
	TransformSystem::TransformSystem()
	{
		//addComponentType<RigidBodyComponent>();
		//addComponentType<Principia::CollisionComponent>();
		addComponentType<TransformComponent>();
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

	void TransformSystem::added(artemis::Entity & e)
	{
		NodeComponent* nc = nodeMapper.get(e);
		//if (nc->isParent) {
			recursiveTransform(nc);
		//}
	}



	void TransformSystem::processEntity(artemis::Entity & e)
	{
		TransformComponent* tc = transformMapper.get(e);
		NodeComponent* nc = nodeMapper.get(e);
		if (nc->isParent && nc->isDynamic)
			 recursiveTransform(nc);
		//size_t numChildren = nc->children.size();
		//for (int c = 0; c < numChildren; c++) {
		//	SQTTransform(nc, tc->local);
		//}
		//if (numChildren == 0 && nc->isParent) {
		//	recursiveTransform(nc);
		//}
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
		if (nc->flags & COMPONENT_PRIMITIVE) {

			PrimitiveComponent* objComp = (PrimitiveComponent*)nc->data->getComponent<PrimitiveComponent>();
			//ssPrimitive& obj = rs->getObject(objComp->objIndex);
			//so what im looking for is the resource manager's

			//scale the aabb
			//obj.center = tc->global.position;
			objComp->extents = tc->global.scale;// rotateAABB(tc->global.rotation, obj.extents * tc->global.scale);
			objComp->world = tc->world;

			rs->setRenderUpdate(RenderSystem::UPDATE_OBJECT);
		}
		else if (nc->flags & COMPONENT_CAMERA) {
			CameraComponent* c = (CameraComponent*)nc->data->getComponent<CameraComponent>();
			//c->pos = tc->global.position;// nodeTrans->position;
			c->rotM = tc->world;
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
	void TransformSystem::regularTransform(NodeComponent* nc, TransformComponent* parent) {
		bool hasParent = nc->parent == nullptr ? false : true;
		TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();

		//if you're a parent, transform based on world coords, else 
		glm::mat4 rotationM;
		glm::mat4 positionM;
		glm::mat4 scaleM;

		//build rotation matrix;
		rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//build position and scale matrix;
		positionM = glm::translate(tc->local.position);
		scaleM = glm::scale(tc->local.scale);

		//combine them into 1 and multiply by parent if u haz parent;
		glm::mat4 local = positionM * rotationM * scaleM;
		hasParent ? tc->world = local : tc->world = local * parent->world;


	}
	void TransformSystem::recursiveTransform(NodeComponent* nc) {
		if (nc->flags & COMPONENT_JOINT)
			return;
		bool hasParent = nc->parent == nullptr ? false : true;
		TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();

		//if you're a parent, transform based on world coords, else 
		glm::mat4 rotationM;
		glm::mat4 positionM;
		glm::mat4 scaleM;

		//build rotation matrix;
		rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//build position and scale matrix;
		positionM = glm::translate(tc->local.position);
		scaleM = glm::scale(tc->local.scale);
		glm::mat4 local = positionM * rotationM;// *scaleM;

		//if (hasParent) {
		//	TransformComponent* pt = (TransformComponent*)nc->parent->data->getComponent<TransformComponent>();
		//	tc->global.scale = tc->local.scale * pt->global.scale;
		//	tc->TRM = tc->world * local;
		//	local = local * scaleM;
		//	tc->world = pt->world * local;
		//}
		//else {
		//	tc->global.scale = tc->local.scale;
		//	tc->TRM = local;
		//	local = local * scaleM;
		//	tc->world = local;
		//}
		hasParent ? tc->global.scale = tc->local.scale * ((TransformComponent*)nc->parent->data->getComponent<TransformComponent>())->global.scale : tc->global.scale = tc->local.scale;

		//combine them into 1 and multiply by parent if u haz parent;
		hasParent ? tc->TRM = ((TransformComponent*)nc->parent->data->getComponent<TransformComponent>())->world * local : tc->TRM = local;
		local = local * scaleM;
		hasParent ? tc->world = ((TransformComponent*)nc->parent->data->getComponent<TransformComponent>())->world * local : tc->world = local;

		if (nc->flags & COMPONENT_PRIMITIVE) {
			//GET THE OBJ
			PrimitiveComponent* objComp = (PrimitiveComponent*)nc->data->getComponent<PrimitiveComponent>();
			//ssPrimitive& obj = rs->getObject(objComp->objIndex);

			//these are primitive shapes, the extents are basically used as the bounds of the shapes
			//if (objComp->uniqueID < 0) {
			//		obj.extents = tc->global.scale; 
			//}
			//else {
			//	//scale the aabb so that if you rotate it, the extents changes accordingly
			//	obj.extents = rotateAABB(tc->global.rotation, tc->global.scale * objComp->extents);
			//	obj.extents = objComp->extents * tc->global.scale;
			//	//put the scale into the matrix for models, but not for shapes
			//	tc->world *= glm::vec4(tc->global.scale, 1.f);
			//	//position the object to be relative to its initial center
			//	glm::vec3 center = objComp->center * tc->global.rotation + tc->global.position;
			//	tc->world[3] = glm::vec4(center, 1.f);
			//}
			objComp->extents = tc->global.scale;
			//obj.invWorld = glm::inverse(tc->TRM);
			//obj.world = tc->world;
			objComp->id < 0 ? objComp->world = tc->TRM : objComp->world = tc->world;
			//objComp->center = tc->world[3];
			//objComp->extents = tc->global.scale;
			//rs->setRenderUpdate(RenderSystem::UPDATE_OBJECT);
		}

		else if (nc->flags & COMPONENT_CAMERA) {
			CameraComponent* c = (CameraComponent*)nc->data->getComponent<CameraComponent>();
			c->rotM = tc->world;
			rs->updateCamera(c);
		}
		else if (nc->flags & COMPONENT_LIGHT) {
			LightComponent* l = (LightComponent*)nc->data->getComponent<LightComponent>();
			ssLight& light = rs->getLight(l->id);
			light.color = l->color;
			light.intensity = l->intensity;
			light.pos = tc->global.position;
			light.pos = tc->world[3];

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
}

/*
bool hasParent = nc->parent == nullptr ? false : true;

	TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();


	glm::mat4 rotationM;
	rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationM = glm::rotate(rotationM, glm::radians(tc->eulerRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	tc->local.rotation = glm::toQuat(rotationM);

	tc->global = tc->local;
	if (hasParent) {
		TransformComponent* ptc = (TransformComponent*)nc->parent->data->getComponent<TransformComponent>();
		tc->global.position = ptc->global.position + tc->local.position;
		//tc->global.position = glm::rotate(ptc->global.rotation, ptc->global.position - tc->local.position);
		//tc->global.position += tc->local.position;
		tc->global.rotation =  tc->local.rotation *  ptc->global.rotation;
		tc->global.scale = ptc->global.scale * tc->local.scale;
	}


	tc->world = glm::toMat4(tc->global.rotation);

	tc->world[3] = glm::vec4(tc->global.position, 1.f);
*/