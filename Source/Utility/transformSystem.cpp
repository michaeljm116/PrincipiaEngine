#include "../pch.h"
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
		addComponentType<HeadNodeComponent>();
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
			//recursiveTransform(nc);
			SQTTransform(nc);
		//}
	}



	void TransformSystem::processEntity(artemis::Entity & e)
	{
		TransformComponent* tc = transformMapper.get(e);
		NodeComponent* nc = nodeMapper.get(e);
		//if (nc->isParent)// && nc->isDynamic)
			SQTTransform(nc);// recursiveTransform(nc);
		//size_t numChildren = nc->children.size();
		//for (int c = 0; c < numChildren; c++) {
		//	SQTTransform(nc, tc->local);
		//}
		//if (numChildren == 0 && nc->isParent) {
		//	recursiveTransform(nc);
		//}
	}

	void TransformSystem::SQTTransform(NodeComponent * nc)
	{
		//Transform errthang...
		TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();
		NodeComponent* pc = nc->parent;
		bool hasParent = (pc != nullptr);

		//Transform matrices
		auto local = glm::translate(glm::vec3(tc->local.position)) *  glm::toMat4(tc->local.rotation);
		auto scaleM = glm::scale(glm::vec3(tc->local.scale));
		
		//combine them into 1 and multiply by parent if u haz parent;
		if (hasParent) {
			auto pt = (TransformComponent*)pc->data->getComponent<TransformComponent>();
			tc->global.scale = tc->local.scale * pt->global.scale;
			tc->global.rotation = tc->local.rotation * pt->global.rotation;
			tc->TRM = pt->world * local;
			local = local * scaleM;
			tc->world = pt->world * local;

		}
		else {
			tc->global.scale = tc->local.scale;
			tc->global.rotation = tc->local.rotation;
			tc->TRM = local;
			local = local * scaleM;
			tc->world = local;
		}

		if (nc->engineFlags & COMPONENT_PRIMITIVE) {
			PrimitiveComponent* objComp = (PrimitiveComponent*)nc->data->getComponent<PrimitiveComponent>();
			objComp->extents = glm::vec3(tc->global.scale);
			objComp->aabbExtents = rotateAABB(glm::mat3(tc->world));
			objComp->id < 0 ? objComp->world = tc->TRM : objComp->world = tc->world;
		}
		else if (nc->engineFlags & COMPONENT_CAMERA) {
			CameraComponent* c = (CameraComponent*)nc->data->getComponent<CameraComponent>();
			c->rotM = tc->world;
			rs->updateCamera(c);
		}
		else if (nc->engineFlags & COMPONENT_LIGHT) {
			LightComponent* l = (LightComponent*)nc->data->getComponent<LightComponent>();
			ssLight& light = rs->getLight(l->id);
			light.color = l->color;
			light.intensity = l->intensity;
			light.pos = glm::vec3(tc->global.position);
			light.pos = tc->world[3];

			rs->setRenderUpdate(RenderSystem::UPDATE_LIGHT);
		}
		if (nc->children.size() > 0) {
			for (NodeComponent* child : nc->children)
			{
				SQTTransform(child);// &tc, rotM, transM, scaleM, false);
			}
		}
	}

	void TransformSystem::recursiveTransform(NodeComponent* nc) {
		if (nc->engineFlags & COMPONENT_JOINT)
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
		tc->local.rotation = rotationM;
		tc->global.rotation *= tc->local.rotation;

		//build position and scale matrix;
		positionM = glm::translate(glm::vec3(tc->local.position));
		scaleM = glm::scale(glm::vec3(tc->local.scale));
		glm::mat4 local = positionM * rotationM;// *scaleM;

		//combine them into 1 and multiply by parent if u haz parent;
		if (hasParent) {
			auto* parent = (TransformComponent*)nc->parent->data->getComponent<TransformComponent>();
			tc->global.scale = tc->local.scale * parent->global.scale;
			tc->TRM = parent->world * local;
			local = local * scaleM;
			tc->world = parent->world * local;
		}
		else {
			tc->global.scale = tc->local.scale;
			tc->TRM = local;
			local = local * scaleM;
			tc->world = local;
		}

		if (nc->engineFlags & COMPONENT_PRIMITIVE) {
			PrimitiveComponent* objComp = (PrimitiveComponent*)nc->data->getComponent<PrimitiveComponent>();
			objComp->extents = glm::vec3(tc->global.scale); 
			objComp->aabbExtents = rotateAABB(glm::mat3(tc->world));
			objComp->id < 0 ? objComp->world = tc->TRM : objComp->world = tc->world;
		}
		else if (nc->engineFlags & COMPONENT_CAMERA) {
			CameraComponent* c = (CameraComponent*)nc->data->getComponent<CameraComponent>();
			c->rotM = tc->world;
			rs->updateCamera(c);
		}
		else if (nc->engineFlags & COMPONENT_LIGHT) {
			LightComponent* l = (LightComponent*)nc->data->getComponent<LightComponent>();
			ssLight& light = rs->getLight(l->id);
			light.color = l->color;
			light.intensity = l->intensity;
			light.pos = glm::vec3(tc->global.position);
			light.pos = tc->world[3];

			rs->setRenderUpdate(RenderSystem::UPDATE_LIGHT);
		}
		if (nc->children.size() > 0) {
			for (NodeComponent* child : nc->children)
			{
				recursiveTransform(child);// &tc, rotM, transM, scaleM, false);
			}
		}
	}

//	glm::vec3 TransformSystem::rotateAABB(const glm::quat & m, const glm::vec3 & extents)
//	{
//
//		//set up cube
//		glm::vec3 v[8];
//		v[0] = extents;
//		v[1] = glm::vec3(extents.x, extents.y, -extents.z);
//		v[2] = glm::vec3(extents.x, -extents.y, -extents.z);
//		v[3] = glm::vec3(extents.x, -extents.y, extents.z);
//		v[4] = glm::vec3(-extents);
//		v[5] = glm::vec3(-extents.x, -extents.y, extents.z);
//		v[6] = glm::vec3(-extents.x, extents.y, -extents.z);
//		v[7] = glm::vec3(-extents.x, extents.y, extents.z);
//
//		//transform them
//#pragma omp parallel for
//		for (int i = 0; i < 8; ++i) {
//			v[i] = abs(m * v[i]);// glm::vec4(v[i], 1.f));
//
//		}
//
//		//compare them
//		glm::vec3 vmax = glm::vec3(FLT_MIN);
//		for (int i = 0; i < 8; ++i) {
//			vmax.x = tulip::max(vmax.x, v[i].x);
//			vmax.y = tulip::max(vmax.y, v[i].y);
//			vmax.z = tulip::max(vmax.z, v[i].z);
//		}
//
//		return vmax;
//	}

	glm::vec3 TransformSystem::rotateAABB(const glm::mat3& m)
	{
		//set up cube
		glm::vec3 extents = glm::vec3(1);
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
		if (nc->engineFlags & COMPONENT_MESH | COMPONENT_MODEL | COMPONENT_BOX)
			return;
		TransformComponent* tc = (TransformComponent*)nc->data->getComponent<TransformComponent>();
		if (nc->engineFlags & COMPONENT_SPHERE) {
			tc->global.scale.y = tc->global.scale.x;
			tc->global.scale.z = tc->global.scale.x;
		}
		if (nc->engineFlags & COMPONENT_CYLINDER) {
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