#include "gridSystem.h"
Principia::GridSystem::GridSystem()
{
	addComponentType<StaticComponent>();
	addComponentType<CollisionComponent>();
}

Principia::GridSystem::~GridSystem()
{
}

void Principia::GridSystem::initialize()
{
	staticMapper.init(*world);
	colMapper.init(*world);

	gridComp = (GridComponent*)world->getSingleton()->getComponent<GridComponent>();
}

void Principia::GridSystem::processEntity(artemis::Entity & e)
{
}

void Principia::GridSystem::added(artemis::Entity & e)
{
	//When you add an entity you get its collision component which has its aabb
	//then you convert that to the grid
	CollisionComponent* cc = colMapper.get(e);
	GridBlock gb = GridBlock(cc->position, cc->extents);
	if (!gb.verify(gridComp->size))
		return;
	for (int r = gb.leftx; r < gb.rightx; ++r) {
		for (int c = gb.downy; c < gb.upy; ++c) {
			if (gridComp->grid[r][c] == nullptr)
				gridComp->grid[r][c] = &e;
		}
	}

	
}

void Principia::GridSystem::removed(artemis::Entity & e)
{
	CollisionComponent* cc = colMapper.get(e);
	GridBlock gb = GridBlock(cc->position, cc->extents);
	if (!gb.verify(gridComp->size))
		return;
	for (int r = gb.leftx; r < gb.rightx; ++r) {
		for (int c = gb.downy; c < gb.upy; ++c) {
			if (gridComp->grid[r][c] != nullptr)
				gridComp->grid[r][c] = nullptr;
		}
	}
}
