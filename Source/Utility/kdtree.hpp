struct KdTree {

	union {
		float split;                  // Interior
		int onePrimitive;             // Leaf
		int primitiveIndicesOffset;   // Leaf
	};
	union {
		int flags;         // Both
		int nPrims;        // Leaf
		int aboveChild;    // Interior
	};
};