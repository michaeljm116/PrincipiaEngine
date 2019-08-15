//#include "sceneLoader.hpp"
#include "kdtree.hpp"
//../../../Assets/Levels/

int main() {
	std::string dir = "../../../Assets/Levels/";
	glm::vec3 bob = glm::vec3(1);
	std::vector<Bounds> transforms = SceneLoader::LoadScene(dir, "Pong/Arena");
	KdTree Bob;
	Bob.build(transforms, 4);
	return 0;
}