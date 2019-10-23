#include "enemyController.h"
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
	if (colMapper.get(e)->state == Principia::CollisionState::Start) {
		e.removeComponent<EnemyComponent>();
		change(e);
	}
}

//Basically when you add an entityi you checkt o see if its the lft
//or hte righ
void EnemyController::added(artemis::Entity & e)
{
	//make sure its not empty
	glm::vec3& pos = transMapper.get(e)->local.position;
	if (leftEnt == nullptr) {
		leftEnt = &e;
		rightEnt = &e;
		leftBounds = &pos.x;
		rightBounds = &pos.x;
	}
	else {
		if (pos.x < *leftBounds) {
			leftBounds = &pos.x;
			leftEnt = &e;
		}
		if (pos.x > *rightBounds) {
			rightBounds = &pos.x;
			rightEnt = &e;
		}		
	}
	//float x = transMapper.get(e)->local.position.x;
	//if (x < leftBounds) leftBounds = x;
	//if (x > rightBounds) rightBounds = x;
	
}

void EnemyController::removed(artemis::Entity & e)
{
	NodeComponent* nc = nodeMapper.get(e);
	for (auto child : nc->children) {
		child->data->remove();
		//world->getEntityManager()->remove(*child->data);
	}
 	//world->getEntityManager()->remove(e);
	e.remove();
}

void EnemyController::begin()
{
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
