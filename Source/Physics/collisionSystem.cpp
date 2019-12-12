#include "collisionSystem.h"
#include "collisionTests.h"
#include "../Utility/helpers.h"

namespace Principia {
	CollisionSystem::CollisionSystem()
	{
		addComponentType<CollisionComponent>();
		addComponentType<TransformComponent>();
		//addComponentType<GameObjectTypeComponent>();
	}

	CollisionSystem::~CollisionSystem()
	{
	}

	void CollisionSystem::initialize()
	{
		colMapper.init(*world);
		transMapper.init(*world);
		//gotMapper.init(*world);
		//grid = (GridComponent*)world->getSingleton()->getComponent<GridComponent>();
	}

	void CollisionSystem::begin()
	{
		//for (auto enemy : enemies) {
		//	for (auto player : players) {
		//		checkCollision(world->getEntity(player), world->getEntity(enemy));// *player, *enemy);
		//	}
		//}
	}

	void CollisionSystem::end()
	{
	}

	void CollisionSystem::added(artemis::Entity & e)
	{
		//GameObjectType t = gotMapper.get(e)->type;
		//if (t & GameObjectType::GAMEOBJECT_ENEMY)
		//	enemies.insert(e.getId());
		//else if (t & GameObjectType::GAMEOBJECT_PLAYER)
		//	players.insert(e.getId());
	}

	void CollisionSystem::removed(artemis::Entity & e)
	{
		//GameObjectType t = gotMapper.get(e)->type;
		//if (t & GameObjectType::GAMEOBJECT_ENEMY)
		//	enemies.erase(e.getId());
		//else if (t & GameObjectType::GAMEOBJECT_PLAYER)
		//	players.erase(e.getId());
	}

	void CollisionSystem::processEntity(artemis::Entity & e)
	{
		//Goes through every object and checks for collision
		TransformComponent* tc = transMapper.get(e);
		CollisionComponent* col = colMapper.get(e);
		col->position = tc->world[3];
		//col->extents = rotateBounds(tc->global.rotation, col->extents);

		//This is for all static objects ///////////////do later right now u finna test spheres
		//checkStaticCollision(e);		

		//This is for all dynamic objects
		//auto a = getActives();
		auto* a = getActives();
		int c = a->getCount();
		const int id = e.getId();
		for (int i = 0; i < c; ++i) {
			auto um = a->get(i);
			if(id != um->getId())
				checkCollision(e, *um);
		}

	}

	void CollisionSystem::checkStaticCollision(artemis::Entity & e)
	{
		std::vector<artemis::Entity*> collisions;
		CollisionComponent* cc = colMapper.get(e);
		GridBlock gb = GridBlock(cc->position, cc->extents);
		if (!gb.verify(grid->size))
			return;
		for (int r = gb.leftx; r < gb.rightx; ++r) {
			for (int c = gb.downy; c < gb.upy; ++c) {
				if (std::find(collisions.begin(), collisions.end(), grid->grid[r][c]) == collisions.end())
					collisions.push_back(grid->grid[r][c]);
			}
		}

		//check collision list for collisions
		for (auto& c : collisions)
			checkCollision(e, *c);
	}

	void CollisionSystem::checkCollision(artemis::Entity & a, artemis::Entity & b)// const
	{
		CollisionComponent* ccA = colMapper.get(a);
		CollisionComponent* ccB = colMapper.get(b);
		glm::vec3 colpos = ccA->position;
		
		if (CollisionTests::ColTests[CollisionTests::Convert(ccA->type, ccB->type)](ccA, ccB, colpos)) {
			CollidedComponent* cwa = (CollidedComponent*)a.getComponent<CollidedComponent>();
			CollidedComponent* cwb = (CollidedComponent*)b.getComponent<CollidedComponent>();

			CollisionData cda = CollisionData(b.getId(), colpos);
			CollisionData cdb = CollisionData(a.getId(), colpos);
			
			cwa == nullptr ?
				a.addComponent(new CollidedComponent(cda))
				:
				cwa->update(cda);

			cwb == nullptr ?
				b.addComponent(new CollidedComponent(cdb))
				:
				cwb->update(cdb);

			a.refresh();
			b.refresh();
		}
	}
	glm::vec3 CollisionSystem::rotateBounds(const glm::quat & m, const glm::vec3 & extents)
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
}