#pragma once

#include "sceneLoader.hpp"
#include <iostream>


struct KdTreeNode {
	float median = 0;
	int axis = 0;
	bool isLeaf = false;
	int level = 0;
	std::shared_ptr<KdTreeNode> child_left;
	std::shared_ptr<KdTreeNode> child_right;
	std::vector<Bounds> prims;

	KdTreeNode(float m, int a) {
		isLeaf = false;
		axis = a;
		median = m;
	}
	KdTreeNode(std::vector<Bounds> p) : prims(p) { isLeaf = true; }

	KdTreeNode() {};
	~KdTreeNode() {/*
		if (child_left)
			child_left.release();
		if (child_right)
			child_right.release();*/
	};

	inline void incrementAxis() {
		axis > 1 ? axis = 0 : axis++;
	}

	void makeLeaf(std::vector<Bounds> p, int& nN, int& nL) {
		nL++;
		prims = std::move(p);
		isLeaf = true;
	}
	void makeNode(std::vector<Bounds> ps, int a, int& nN, int& nL) {
		nN++;
		isLeaf = false;
		axis = a;

		//set up the median
		median = 0;
		for (const auto& p : ps)
			median += p.center[axis];
		median /= (float)ps.size();

		//set up the bounds
		std::vector<Bounds> leftBounds;
		std::vector<Bounds> rightBounds;
		for (const auto& p : ps) {
			p.center[axis] < median ?
				leftBounds.push_back(p) :
				rightBounds.push_back(p);
		}
		incrementAxis();

		//recurse
		if (leftBounds.size() > 0) {
			child_left = std::make_shared<KdTreeNode>();
			leftBounds.size() < 3 ? child_left.get()->makeLeaf(leftBounds, nN, nL) :
				child_left.get()->makeNode(leftBounds, axis, nN, nL);
		}
		else {makeLeaf(rightBounds, nN, nL); return; }
		if (rightBounds.size() > 0) {
			child_right = std::make_shared<KdTreeNode>();
			rightBounds.size() < 3 ? child_right.get()->makeLeaf(rightBounds, nN, nL) :
				child_right.get()->makeNode(rightBounds, axis, nN, nL);
		}
		else {makeLeaf(leftBounds, nN, nL); return; }

	}
};

struct KdTree {

	std::shared_ptr<KdTreeNode> root;
	int axis = 1;
	int numNodes = 0;
	int numLeaves = 0;
	int maxDepth = 0;

	void build(std::vector<Bounds> prims) {
		root = std::make_shared<KdTreeNode>();
		root.get()->makeNode(prims, axis, numNodes, numLeaves);
		int a = 4;
	}

	//int maxDepth(std::shared_ptr<KdTreeNode> n) {
	//	if (n == nullptr)
	//		return 0;

	//}
	
};