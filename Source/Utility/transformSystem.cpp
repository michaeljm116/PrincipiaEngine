#include "../pch.h"
#include "transformSystem.h"
#include "../Rendering/renderSystem.h"
#include "../Physics/Components/collisionComponent.h"
#include "helpers.h"
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

#define UIIZON

namespace Principia {
	TransformSystem::TransformSystem()
	{
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
		rs = (RenderSystem*)world->getSystemManager()->getSystem<RenderSystem>();

	}

	void TransformSystem::added(artemis::Entity & e)
	{
		NodeComponent* nc = nodeMapper.get(e);
		SQTTransform(nc);
	}

	auto display_transform = [](NodeComponent* n, TransformComponent* t) {
		std::cout << "\nEntity Name: " << n->name << std::endl;
		std::cout << "Transform Details:" << std::endl;

		glm::vec3 euler_angles = glm::degrees(glm::eulerAngles(t->local.rotation));

		std::cout << "Rotation: (" << euler_angles.x << ", " << euler_angles.y << ", " << euler_angles.z << ")" << std::endl;
		std::cout << "Position: (" << t->local.position.x << ", " << t->local.position.y << ", " << t->local.position.z << ")" << std::endl;
		std::cout << "Scale: (" << t->local.scale.x << ", " << t->local.scale.y << ", " << t->local.scale.z << ")" << std::endl;
	};


	void TransformSystem::processEntity(artemis::Entity & e)
	{
		SQTTransform(nodeMapper.get(e));
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

#ifdef UIIZON
		tc->eulerRotation = glm::eulerAngles(tc->local.rotation);
		tc->eulerRotation = glm::vec3(glm::degrees(tc->eulerRotation.x), glm::degrees(tc->eulerRotation.y), glm::degrees(tc->eulerRotation.z));
#endif // UIIZON

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

			rs->setRenderUpdate(Renderer::kUpdateLight);
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
		glm::mat4 rotationM = glm::mat4(1.f);
		glm::mat4 positionM = glm::mat4(1.f);
		glm::mat4 scaleM = glm::mat4(1.f);

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

			rs->setRenderUpdate(Renderer::kUpdateLight);
		}
		if (nc->children.size() > 0) {
			for (NodeComponent* child : nc->children)
			{
				recursiveTransform(child);// &tc, rotM, transM, scaleM, false);
			}
		}
	}

	glm::vec3 TransformSystem::rotateAABB(const glm::mat3& m)
	{
		//set up cube
		glm::vec3 extents = glm::vec3(1);
		glm::vec3 v[8] = 
		{ 
			glm::vec3(extents),
			glm::vec3(extents.x, extents.y, -extents.z),
			glm::vec3(extents.x, -extents.y, -extents.z),
			glm::vec3(extents.x, -extents.y, extents.z),
			glm::vec3(-extents),
			glm::vec3(-extents.x, -extents.y, extents.z),
			glm::vec3(-extents.x, extents.y, -extents.z),
			glm::vec3(-extents.x, extents.y, extents.z)
		};	

		//transform them
//#pragma omp parallel for
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
