#include "structs.h"

struct KdTreeNode {
	float split = 0;
	int axis = 0;
	bool isLeaf = false;
	int level = 0;
	KdTreeNode* child_left;
	KdTreeNode* child_right;
	std::vector<int> faceIndexes;
};

struct FlatKdNode {
	union {
		float split;
		int onePrim;
		int primOffset;
	};
	union {
		int flags;
		int numPrims;
		int aboveChild;
	};

	FlatKdNode(std::unordered_map<int, int>& comp, std::vector<int>& orderedFaces, KdTreeNode* node) {

		if (node->isLeaf) {
			//first check to see this hasn't been already pushed into the tree
			for (auto fi : node->faceIndexes) {
				if (comp.find(fi) == comp.end()) {	//aka it hasn't been used previously
					orderedFaces.push_back(fi);		//so push it into the list of ordered faces
					comp[fi] = orderedFaces.size(); //and then mark it on the map
				}
			}
			size_t size = node->faceIndexes.size();

			//If its a leaf node make a leaf node
			if (size == 1) {
				onePrim = node->faceIndexes[0];
			}
			//still a leaf node
			else if (size > 1) {
				primOffset = orderedFaces.size();
				numPrims = size;
			}
			//must be interior then
			else {
				split = node->split;
				flags = setFlags(node->axis);
			}
		}
	}
	int setFlags(int axis) {
		switch (axis) {
		case 0: //x
			return 1 << 29;
			break;
		case 1: //y
			return 2 << 29;
			break;
		case 2: //z
			return 3 << 29;
			break;
		}
	}

	int setRightChild(int rc) {
		int flag = flags >> 29;
		aboveChild = rc;
		aboveChild |= flag;
	}

};

static void buildKdTree(Mesh& mesh) {
	//Structures for an easy to understand build of the tree
	KdTreeNode* root;

	//Structures to flatten the tree
	std::unordered_map<int, int> um_visited;
	std::vector<int> v_orderedFaceIndexes;
	std::vector<FlatKdNode*> v_flatTree;
	


}

FlatKdNode* flattenKDTree(std::vector<FlatKdNode*>& tree, std::unordered_map<int, int>& visited, std::vector<int>& ofi, KdTreeNode* n) {
	FlatKdNode* fn = new FlatKdNode(visited, ofi, n);
	tree.push_back(fn);
	if(!n->isLeaf) {
		if (n->child_left != nullptr) {
			tree.push_back(flattenKDTree(tree, visited, ofi, n->child_left));
		}
		if (n->child_right != nullptr) {
			tree.push_back(flattenKDTree(tree, visited, ofi, n->child_right));
			fn->setRightChild(tree.size());
		}
	}
	
}