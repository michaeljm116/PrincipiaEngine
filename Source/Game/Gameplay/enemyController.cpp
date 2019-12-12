#include "enemyController.h"
#include <algorithm>
#pragma once

EnemyController::EnemyController()
{
	addComponentType<EnemyComponent>();
	addComponentType<Principia::CollisionComponent>();
	addComponentType<TransformComponent>();
	addComponentType<NodeComponent>();
}

EnemyController::~EnemyController()
{
}

void EnemyController::initialize()
{
	enemyMapper.init(*world);
	colMapper.init(*world);
	transMapper.init(*world);
	nodeMapper.init(*world);
}

void EnemyController::processEntity(artemis::Entity & e)
{
	//move the dude
	float& x = transMapper.get(e)->local.position.x;
	float& z = transMapper.get(e)->local.position.z;

	x += hstep;
	z += vstep * bhmul;
	//update bounds


	//destroy if hit something
	//if (colMapper.get(e)->state == Principia::CollisionState::Enter) {
	//	e.removeComponent<EnemyComponent>();
	//	change(e);
	//}
}

//Basically when you add an entityi you checkt o see if its the lft
//or hte righ
void EnemyController::added(artemis::Entity & e)
{
	//make sure its not empty
	TransformComponent* tc = transMapper.get(e);
	pmap[e.getId()] = positions.size();
	positions.push_back(std::pair<glm::vec3*, int>(&tc->local.position, e.getId()));
	heapify = true;
	//glm::vec3& pos = transMapper.get(e)->local.position;
	//if (leftEnt == nullptr) {
	//	leftEnt = &e;
	//	rightEnt = &e;
	//	leftBounds = &pos.x;
	//	rightBounds = &pos.x;
	//}
	//else {
	//	if (pos.x < *leftBounds) {
	//		leftBounds = &pos.x;
	//		leftEnt = &e;
	//	}
	//	if (pos.x > *rightBounds) {
	//		rightBounds = &pos.x;
	//		rightEnt = &e;
	//	}		
	//}


	//float x = transMapper.get(e)->local.position.x;
	//if (x < leftBounds) leftBounds = x;
	//if (x > rightBounds) rightBounds = x;
	
}

void EnemyController::removed(artemis::Entity & e)
{
	int index = pmap[e.getId()];
	positions[index].second = -1;
	NodeComponent* nc = nodeMapper.get(e);
	for (auto child : nc->children) {
		child->data->remove();
		child->data->refresh();
		//world->getEntityManager()->remove(*child->data);
	}
 	//world->getEntityManager()->remove(e);
	e.remove();
	e.refresh();
	if (leftEnt == rightEnt) {
		//GAME OVER YOU WIN WOOOOOOO
	}
	
	//this checks to see if the left or right most entities have been destroyed
	//if so then get the nnext closest one
	else {
		std::pair<glm::vec3*, int> lp = positions[leftEnt];
		std::pair<glm::vec3*, int> rp = positions[rightEnt];

		while (lp.second < 0 && leftEnt <= rightEnt) {
 			leftEnt++;
			lp = positions[leftEnt];
		}
		while (rp.second < 0 && rightEnt >= leftEnt) {
			rightEnt--;
			rp = positions[rightEnt];
		}

		leftBounds = &lp.first->x;
		rightBounds = &rp.first->x;

	}
	
}

void EnemyController::begin()
{
	if (heapify) {
		heapify = false;

		auto lesser = [](std::pair<glm::vec3*, int> a, std::pair<glm::vec3*, int>b)->bool {
			if (a.first->x == a.first->x)
				return a.first->z < a.first->z;
			return a.first->x < a.first->x;
		};
		std::sort(positions.begin(), positions.end(), lesser);

		leftEnt = 0;
		rightEnt = positions.size() - 1;

		leftBounds = &positions[leftEnt].first->x;
		rightBounds = &positions[rightEnt].first->x;
	}
	//do a single move with the left and right most bound
	//if it goes beyond the stage then floor and flag
	float nextStep = world->getDelta() * speed * dir;
	if (dir > 0) {
		float nextRight = *rightBounds + nextStep;

 		if (nextRight > stageRightBounds) {
			boundHit = true;
			nextStep = nextRight - stageRightBounds;// (*rightBounds + nextStep - stageRightBounds;
		}
	}
	else {
		float nextLeft = *leftBounds + nextStep;
		if (nextLeft < stageLeftBounds) {
			boundHit = true;
			nextStep = nextLeft - stageLeftBounds;
		}
	}
	hstep = nextStep;
}

void EnemyController::end()
{
	if (boundHit) {
		bhmul = 1.f;
		dir *= -1.f;
		boundHit = false;
	}
	else {
		bhmul = 0.f;
	}
}
