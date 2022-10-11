#include "test_scene.h"
#include "Utility/serialize-node.h"
#include "../tinyxml2/tinyxml2.h"
#include <assert.h>

test::TestScene::TestScene(std::string&& s) {
	LoadScene(s);
}

test::TestScene::~TestScene()
{
}

void test::TestScene::LoadScene(std::string s) {
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError e_result = doc.LoadFile(s.c_str());
	assert(e_result == tinyxml2::XMLError::XML_SUCCESS);

	tinyxml2::XMLNode* p_node = doc.FirstChild();
	tinyxml2::XMLElement* p_root = doc.FirstChildElement("Root");
	tinyxml2::XMLElement* scene_num = p_root->FirstChildElement("Scene");
	
	int scene_number = 0;
	p_root->FirstChildElement("Scene")->QueryIntAttribute("Num", &scene_number);

	tinyxml2::XMLElement* first = p_root->FirstChildElement("Node");
	tinyxml2::XMLElement* last = p_root->LastChildElement("Node");
	parents = Principia::SERIALIZENODE.loadNodes(first, nullptr);

}