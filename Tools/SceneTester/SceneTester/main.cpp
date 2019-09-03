//#include "sceneLoader.hpp"
#include "bvh.hpp"
//../../../Assets/Levels/

int main() {
	int a = 3;
	int b = 55;

	a = a << 28;
	b = b | a;
	std::cout << b;
	int c = a >> 30;
	int d = a << 2;
	int e = d >> 2;

	std::cout << "\n" << c << " " << e;

	/*
	
	*/

	///_______________________________________________________________________________________________________________________________________________________________________
	//std::string dir = "../../../Assets/Levels/";
	//glm::vec3 bob = glm::vec3(1);
	//std::vector<Bounds> transforms = SceneLoader::LoadScene(dir, "Pong/Arena");
	//KdTree Bob;
	//Bob.build(transforms);

	//BVHTree Bill;
	//Bill.build(SplitMethod::Middle, TreeType::Recursive, transforms);
	//Bill.flattenBVH();
	///_______________________________________________________________________________________________________________________________________________________________________
	//float tester = 0.f;
	//Bounds *midPtr = std::partition(&transforms[0], &transforms[transforms.size()-1] + 1, [tester](const Bounds& b) {
	//	return b.center.x < tester;
	//});
	//int mid = midPtr - &transforms[0];

	system("pause");
	return 0;
}