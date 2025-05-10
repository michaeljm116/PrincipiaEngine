#pragma once
#include <Artemis/Component.h>
#include <string>

struct PrefabComponent : public artemis::Component
{
	std::string name = "";
	std::string dir = "";
	bool save = false;				//If true, save the prefab
	bool load_needed = false;		//If true, Load prefab on start-up
	bool can_serialize = false;		//If true, Allow the prefab to saved 
	PrefabComponent(const std::string& n, const std::string& d, bool s) : name(n), dir(d), save(s) {};
	PrefabComponent(const std::string& n, const std::string& d, bool s, bool l, bool cs) : name(n), dir(d), save(s), load_needed(l), can_serialize(cs) {};
	PrefabComponent() {};
};