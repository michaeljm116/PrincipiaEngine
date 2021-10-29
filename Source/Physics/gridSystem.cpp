#include "gridSystem.h"
#include "Components/immovableComponent.h"
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

	//gridComp = (GridComponent*)world->getSingleton()->getComponent<GridComponent>();
}

void Principia::GridSystem::processEntity(artemis::Entity & e)
{
}

void Principia::GridSystem::added(artemis::Entity & e)
{
	e.addComponent(new ImmovableComponent());
	//When you add an entity you get its collision component which has its aabb
	//then you convert that to the grid


	CollisionComponent* cc = colMapper.get(e);
	//GridBlock gb = GridBlock(cc->position, cc->extents);
	//if (!gb.verify(gridComp->size))
	//	return;
	//for (int r = gb.leftx; r < gb.rightx; r += gb.itr) {
	//	for (int c = gb.downy; c < gb.upy; c += gb.itr) {
	//		if (gridComp->grid[r][c] == nullptr)
	//			gridComp->grid[r][c] = &e;
	//	}
	//}

	printGrid();

	
}

void Principia::GridSystem::removed(artemis::Entity & e)
{
	CollisionComponent* cc = colMapper.get(e);
	//GridBlock gb = GridBlock(cc->position, cc->extents);
	//if (!gb.verify(gridComp->size))
	//	return;
	//for (int r = gb.leftx; r < gb.rightx; r += gb.itr) {
	//	for (int c = gb.downy; c < gb.upy; c += gb.itr) {
	//		if (gridComp->grid[r][c] != nullptr)
	//			gridComp->grid[r][c] = nullptr;
	//	}
	//}
}

void Principia::GridSystem::printGrid()
{
	std::cout << "\n";
	//for (int r = 0; r < gridComp->size.x; ++r) {
	//	for (int c = gridComp->size.y - 1; c >= 0; --c) {
	//		if (gridComp->grid[r][c] == nullptr)
	//			std::cout << "0";
	//		else
	//			std::cout << "1";
	//	}
	//	std::cout << "\n";
	//}
}
