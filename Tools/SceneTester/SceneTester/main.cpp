//#include "sceneLoader.hpp"
#include "bvh.hpp"
//../../../Assets/Levels/

#include <vector>
#include <unordered_map>
void reverseString(std::vector<char>& s) {
	int half = s.size() >> 1;
	int j = s.size() - 1;
	for (int i = 0; i < half; ++i, j--) {
		std::swap(s[i], s[j]);
	}
}

//int a = 2;
//int b = 55;
//int m = INT32_MAX;
//m = m >> 3;
//std::cout << "M: " << m << std::endl;

//int bo3m = 268435455;
//a = a << 29;
//b = b | a;
//std::cout << b;
//int c = b >> 29;
//int d = b << 3;
//int e = d >> 3;
//e = b & m;

//c = 0;
//c |= 3 << 29;
//c = c & bo3m;
//d = 2; 
//d |= 2 << 29;
//d = d & bo3m;
//e = 1;
//e |= a;
//e = e & bo3m;
//

//std::cout << "\n THE RESULTS AREEEEE: " << c << " " << d << " " << e << std::endl;

//int sign;
//int v = -2;

//sign = 1 ^ ((unsigned int)v >> 31);
//std::cout << "\n some mo results areeee: " << sign;

//v = 3;
//sign = 1 ^ ((unsigned int)v >> 31);
//std::cout << "\n some mo results areeee: " << sign;



////if the first bit is 1 its negative
//glm::ivec3 dneg = glm::ivec3(-1 < 0, 2 < 0, 0 < 0);
//std::cout << "\n the other results are::: " << dneg.x << dneg.y << dneg.z;
//std::vector<char> cho;
//cho.end();

//std::vector<char> bob = { 'h', 'e', 'l', 'l', 'o' };
//reverseString(bob);
//for (int i = 0; i < bob.size(); ++i) {
//	std::cout << bob[i];
//}

int main() {
	
	int bm= 0;
	bm = 1 << 0;
	bm = 1 << 1;
	bm = 1 << 2;
	bm = 1 << 3;
	bm = 1 << 4;
	bm = 3 << 1;
	bm = 3 << 2;
	bm = 3 << 4;


	
	std::string dir = "../../../Assets/Levels/";
	glm::vec3 bob = glm::vec3(1);
	std::vector<Bounds> transforms = SceneLoader::LoadScene(dir, "Pong/Arena");
	KdTree Bob;
	Bob.build(transforms);

	BVHTree Bill;
	Bill.build(SplitMethod::SAH, TreeType::Recursive, transforms);
	Bill.flattenBVH();
	///_______________________________________________________________________________________________________________________________________________________________________
	//float tester = 0.f;
	//Bounds *midPtr = std::partition(&transforms[0], &transforms[transforms.size()-1] + 1, [tester](const Bounds& b) {
	//	return b.center.x < tester;
	//});
	//int mid = midPtr - &transforms[0];
	int size = transforms.size();
	int mid = size >> 1;
	int dim = 0;
	std::nth_element(&transforms[0], &transforms[mid], &transforms[size - 1] + 1, [dim](const Bounds& a, const Bounds& b) {
		return a.center[dim] < b.center[dim];
	});
	
	system("pause");
	return 0;
}