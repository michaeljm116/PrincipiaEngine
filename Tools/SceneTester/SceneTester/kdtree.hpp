#pragma once

#include "sceneLoader.hpp"
#include <iostream>


struct KdTreeNode {
	float median = 0;
	int axis = 0;
	bool isLeaf = false;
	std::unique_ptr<KdTreeNode> child_left;
	std::unique_ptr<KdTreeNode> child_right;
	std::vector<Bounds> prims;

	KdTreeNode(float m, int a) {
		isLeaf = false;
		axis = a;
		median = m;
	}
	KdTreeNode(std::vector<Bounds> p) : prims(p) { isLeaf = true; }

	KdTreeNode() {};
	~KdTreeNode() {
		if (child_left)
			child_left.release();
		if (child_right)
			child_right.release();
	};

	inline void setLeft(std::unique_ptr<KdTreeNode> node) {
		child_left = std::move(node);
	}

};

class KdTree {
public:
	void build(std::vector<Bounds> prims, int a) {
		float median = 0;
		for (const auto& p : prims)
			median += p.center[axis];
		median /= (float)prims.size();

		//sort by median
		std::vector<Bounds> leftBounds;
		std::vector<Bounds> rightBounds;
		for (const auto& p : prims) {
			p.center[axis] < median ?
				leftBounds.push_back(p) :
				rightBounds.push_back(p);
		}

		//make new node
		std::unique_ptr<KdTreeNode> root = std::make_unique<KdTreeNode>(median, axis);
		IncrementAxis();
		root->child_left = build(leftBounds);
		root->child_right = build(rightBounds);
	}
private:
	std::unique_ptr<KdTreeNode> root;
	int axis = 1;
	std::unique_ptr<KdTreeNode> build(std::vector<Bounds> prims) {
		if (prims.size() < 1)
			return nullptr;
		if (prims.size() < 3) {
			return std::make_unique<KdTreeNode>(prims);
		}
		else {
			//get the median
			float median = 0;
			for (const auto& p : prims) 
				median += p.center[axis];
			median /= (float)prims.size();

			//sort by median
			std::vector<Bounds> leftBounds;
			std::vector<Bounds> rightBounds;
			for (const auto& p : prims) {
				p.center[axis] < median ?
					leftBounds.push_back(p) :
					rightBounds.push_back(p);
			}

			//make new node
			std::unique_ptr<KdTreeNode> node = std::make_unique<KdTreeNode>(median, axis);
			IncrementAxis();
			node->child_left = build(leftBounds);
			node->child_right = build(rightBounds);

			return node;
		}
	}

	inline void IncrementAxis() {
		axis > 1 ? axis = 0 : axis++;
	}
};