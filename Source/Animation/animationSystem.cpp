#include "animationSystem.h"
#include "../Utility/resourceManager.h"
#include "../Utility/Input.h"
#include "../pch.h"
#pragma once



AnimationSystem::AnimationSystem()
{
	addComponentType<AnimationComponent>();
	addComponentType<NodeComponent>();
	addComponentType<TransformComponent>();
}


AnimationSystem::~AnimationSystem()
{
}

void AnimationSystem::initialize()
{
	nodeMapper.init(*world);
	animationMapper.init(*world);
	transformMapper.init(*world);

	em = world->getEntityManager();
	sm = world->getSystemManager();
	rs = (RenderSystem*)sm->getSystem<RenderSystem>();
}

void AnimationSystem::processEntity(artemis::Entity & e)
{
	AnimationComponent* ac = animationMapper.get(e);
	NodeComponent* nc = nodeMapper.get(e);
	TransformComponent* tc = transformMapper.get(e);

	rSkeleton* skele = &RESOURCEMANAGER.getSkeleton(ac->skeleton.index);
	rAnimation* anim = &skele->animations[ac->animIndex];//&RESOURCEMANAGER.getSkeleton(ac->skeleton.index).animations[ac->animIndex];
	//Input::Timer timer(anim->name.c_str());
	//find a way to increment time;
	//So there's glike a global time thing already but each animation should track its own time but should it be an ever looping time or just like time based off start of new animation?!?
	//right now just do time based on new animation
	ac->time += world->getDelta();
	if (ac->time > anim->duration / anim->sps) {
		ac->time = 0;// -= anim->duration / anim->sps;
		for (int i = 0; i < anim->numChannels; ++i) {
			ac->channels[i].key_current = 0;
			ac->channels[i].key_next = 1;
		}
	}

	/*
	okay so at start of animation, current = 0, next = 1
	to check for next you
		look at annimaiton->channel->key
		so for each channel,you compare the ac time to the key time, if its above nextkey time, you switch current key to next key and next key
		and then set nextkey to be % num keys
		the time duration thing should make it so that it gets reset
	*/
	
	//u iz finna interpolizzate up in dis hurr systym
	for (int i = 0; i < anim->numChannels; ++i) {
		int pi = ac->skeleton.joints[i]->parentIndex;

		//get the next key
		if (ac->time > anim->channels[i].keys[ac->channels[i].key_next].time / anim->sps) {
			ac->channels[i].key_current = ac->channels[i].key_next;
			ac->channels[i].key_next = (ac->channels[i].key_current + 1) % anim->channels[i].keys.size();
		}
		rAnimKey* currentKey = &anim->channels[i].keys[ac->channels[i].key_current];
		rAnimKey* nextKey = &anim->channels[i].keys[ac->channels[i].key_next];
		float delta = (ac->time - currentKey->time) / (nextKey->time / currentKey->time);

		//Transforms stuff
		//glm::vec3 pos = currentKey->pos + ((nextKey->pos - currentKey->pos) * delta);
		//currentKey->rot = glm::normalize(currentKey->rot);
		//glm::quat rot = Interpolate(currentKey->rot, nextKey->rot, delta);
		//glm::vec3 sca = currentKey->sca + ((nextKey->sca - currentKey->sca) * delta);
		
		glm::mat4 world = glm::toMat4(currentKey->rot) * glm::scale(currentKey->sca);
		world[3] = glm::vec4(currentKey->pos, 1.f);
		//world[3] = glm::vec4(currentKey->pos - (currentKey->pos - ac->skeleton.joints[i]->center), 1.f);


		bool animon = ac->on;
		//animon = true;
		
		//ac->channels[i].combined = glm::translate(pos) * glm::toMat4(rot) * glm::scale(sca);
		//glm::mat3 bob = glm::toMat3(currentKey->rot);
		if (animon) {
			ac->channels[i].combined = world;// glm::translate(currentKey->pos) * glm::toMat4(currentKey->rot) * glm::scale(currentKey->sca);
			//ac->channels[i].combined = glm::translate(pos) * glm::toMat4(rot) * glm::scale(sca);
			if (pi > -1)
				ac->skeleton.joints[i]->global_Transform = ac->skeleton.joints[pi]->global_Transform * ac->skeleton.joints[i]->invBindPose *   ac->channels[i].combined;
			else
				ac->skeleton.joints[i]->global_Transform = ac->skeleton.joints[i]->invBindPose * ac->channels[i].combined;

			//
			ac->skeleton.joints[i]->global_Transform[3] = ac->skeleton.joints[i]->global_Transform[3] - (ac->skeleton.joints[i]->global_Transform[3] - glm::vec4(ac->skeleton.joints[i]->center, 1.f));
			ac->skeleton.joints[i]->final_Transform = tc->world * ac->skeleton.joints[i]->global_Transform;// *skele->globalInverseTransform;// *tc->worldM;
		}
		else {
			if (pi > -1)
				ac->skeleton.joints[i]->global_Transform = ac->skeleton.joints[pi]->global_Transform * ac->skeleton.joints[i]->transform;
			else
				ac->skeleton.joints[i]->global_Transform = ac->skeleton.joints[i]->transform;// *skele->globalInverseTransform;//skele->globalInverseTransform * ac->skeleton.joints[i]->transform;
			//ac->skeleton.joints[i]->final_Transform = tc->world * ac->skeleton.joints[i]->global_Transform;
			ac->skeleton.joints[i]->final_Transform = tc->world * ac->skeleton.joints[i]->transform;//ac->skeleton.joints[i]->global_Transform;// *skele->globalInverseTransform;
		}
	}

	for (auto joint : ac->skeleton.joints) {
		ssJoint* j = &rs->getJoint(joint->renderIndex);
		j->world = joint->final_Transform * glm::scale(joint->extents);
		j->extents = tc->global.scale * joint->extents;
	}
	rs->setRenderUpdate(RenderSystem::UPDATE_JOINT);

	//aiight now get all dem uhhhh wwwwwwwwwwwwwwow this is gonna suck
//	for (int i = 0; i < nc->children.size(); ++i)
//	{
////		//TransformComponent* nodeTrans = (TransformComponent*)nc->data->getComponent<TransformComponent>();
////		MeshComponent* sub = (MeshComponent*)nc->children[i]->data->getComponent<MeshComponent>();
////		std::vector<ssVert>& verts = rs->getVertices();
////		ssMesh& mesh = rs->getMesh(sub->meshIndex);
////		rMesh& rSub = RESOURCEMANAGER.getModelU(sub->meshModelID).meshes[sub->meshResourceIndex];
////
////		//mesh.center = glm::vec3(tc->world * glm::vec4(rSub.center, 1.f));
////		//mesh.extents = rotateAABB(tc->global.rotation, rSub.extents * tc->global.scale);//rotateAABB(glm::mat3(tc->scaleM * tc->rotationM), rSub.extents);
////
////		int start = mesh.startVert;
////		int end = mesh.endVert + 1;
////
////#pragma omp parallel for
////		for (int i = start; i < end; ++i) {
////			glm::mat4 boneTrans = tc->world * BoneTransform(ac->skeleton, rSub.bones[i - start]);
////			//glm::mat4 boneTrans = BoneTransform(ac->skeleton, rSub.bones[i - start]);
////			verts[i].pos = glm::vec3(boneTrans * glm::vec4(rSub.verts[i - start], 1.f));
////		}
//		
//		rs->setRenderUpdate(RenderSystem::UPDATE_OBJECT);
//	}
}

void AnimationSystem::addNode(NodeComponent * node)
{

}

void AnimationSystem::update(float dt)
{
	deltaTime = abs(dt);
	globalTime += deltaTime;
}


glm::quat AnimationSystem::Interpolate(glm::quat start, glm::quat end, float delta)
{
	///////////////////////// MAKE IT NOT SUCK LATER ///////////////////////////
	float t = 1 - delta;
	glm::quat ret;
#pragma omp parallel for
	for (int i = 0; i < 4; ++i)
	{
		ret[i] = t * start[i] + t * end[i];
	}
	ret = glm::normalize(ret);
	return ret;
}

glm::mat4 AnimationSystem::BoneTransform(const Skeleton & skeleton, const rJointData & joints)
{
	//glm::mat4 boneTrans = glm::mat4(0);
	//for (int i = 0; i < 4; ++i) {
	//	float weight = joints.weights[i];
	//	if (weight > BONE_EPSILON) {
	//		boneTrans += skeleton.joints[joints.id[i]].final_Transform * weight;
	//	}
	//}
	////boneTrans = skeleton.joints[joints.id[0]].final_Transform;
	//return boneTrans;
	return glm::mat4();
}

glm::vec3 AnimationSystem::rotateAABB(const glm::quat & m, const glm::vec3 & extents)
{
	//set up cube
	glm::vec3 v[8];
	v[0] = extents;
	v[1] = { extents.x, extents.y, -extents.z };
	v[2] = { extents.x, -extents.y, -extents.z };
	v[3] = {extents.x, -extents.y, extents.z};
	v[4] = {-extents};
	v[5] = {-extents.x, -extents.y, extents.z};
	v[6] = {-extents.x, extents.y, -extents.z};
	v[7] = {-extents.x, extents.y, extents.z};

	//transform them
#pragma omp parallel for
	for (int i = 0; i < 8; ++i) {
		v[i] = abs(m * v[i]);

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

/*
#pragma region TESTY
		if (pi > -1) {
			ac->channels[i].combinedPos = ac->channels[pi].combinedPos + currentKey->pos;
			ac->channels[i].combinedSca = ac->channels[pi].combinedSca + currentKey->sca;
			ac->channels[i].combinedRot = ac->channels[pi].combinedRot * currentKey->rot;
		}
		else {
			ac->channels[i].combinedPos = currentKey->pos;
			ac->channels[i].combinedSca = currentKey->sca;
			ac->channels[i].combinedRot = currentKey->rot;
		}
		ac->channels[i].combined = glm::translate(ac->channels[i].combinedPos) * glm::toMat4(ac->channels[i].combinedRot) * glm::scale(ac->channels[i].combinedSca);

		if (pi > -1)
			ac->skeleton.joints[i]->global_Transform = ac->skeleton.joints[pi]->global_Transform * ac->channels[i].combined;
		else
		ac->skeleton.joints[i]->global_Transform = ac->channels[i].combined;

		ac->skeleton.joints[i]->final_Transform = ac->skeleton.joints[i]->global_Transform * ac->skeleton.joints[i]->invBindPose;// *tc->worldM;

#pragma region ENDTESTY

*/