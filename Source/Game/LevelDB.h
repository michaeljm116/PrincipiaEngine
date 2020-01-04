#include <unordered_map>
#include <iostream>
#include "../../Lib/xxhash.hpp"
typedef std::unordered_map<std::string, std::pair<std::string, std::string>> SceneDatabase;
static SceneDatabase g_SceneDatabase;
static inline xxh::hash32_t xxhasher(std::string s) { return xxh::xxhash<32, char>(s.c_str(), 0); }
//static uint32_t s_AllocCount = 0;
//void* operator new(size_t size) {
//	s_AllocCount++;
//	std::cout << "Allocating: " << size << "bytes \n";
//	return malloc(size);
//}
static void CreateDatabase() {
	g_SceneDatabase.emplace("TestEntrance", std::make_pair("../../Assets/Levels/Test/", "TestEntrance.xml"));
}

