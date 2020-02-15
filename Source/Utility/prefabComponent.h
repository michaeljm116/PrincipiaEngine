#pragma once
#include <Artemis/Component.h>
#include <string>

struct PrefabComponent : public artemis::Component
{
	std::string name;
	std::string dir;
	bool save = false;
	PrefabComponent(const std::string& n, const std::string& d, bool s) : name(n), dir(d), save(s) {};
	PrefabComponent() {};
};