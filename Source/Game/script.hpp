#pragma once
#include "../tinyxml2/tinyxml2.h"
#include <vector>
#include <glm\glm.hpp>


#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != tinyxml2::XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

class Scripto {

public:
	Scripto() {};
	~Scripto() {};
	
	std::vector<glm::vec3> vData;
	std::vector<float> fData;
	std::vector<int> iData;
	std::vector<std::string> sData;

	int numVecs;
	int numFloats;
	int numInts;
	int numStrings;

	tinyxml2::XMLError loadScript(const char* file) {
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError eResult = doc.LoadFile(file);
		XMLCheckResult(eResult);

		tinyxml2::XMLNode * proot = doc.FirstChild();
		tinyxml2::XMLElement * pRoot = doc.FirstChildElement("Data");
		tinyxml2::XMLElement* initScript;

		initScript = pRoot->FirstChildElement("NumVecs");
		initScript->QueryIntAttribute("n", &numVecs);
		initScript = pRoot->FirstChildElement("NumFloats");
		initScript->QueryIntAttribute("n", &numFloats);
		initScript = pRoot->FirstChildElement("NumInts");
		initScript->QueryIntAttribute("n", &numInts);
		initScript = pRoot->FirstChildElement("NumStrings");
		initScript->QueryIntAttribute("n", &numStrings);

		tinyxml2::XMLElement* dataElement;
		if (numVecs) {
			initScript = pRoot->FirstChildElement("Vecs");
			dataElement = initScript->FirstChildElement("vec");
			for (int i = 0; i < numVecs; ++i) {
				if (i > 0)
					dataElement = dataElement->NextSiblingElement("vec");
				glm::vec3 data;
				dataElement->QueryFloatAttribute("x", &data.x);
				dataElement->QueryFloatAttribute("y", &data.y);
				dataElement->QueryFloatAttribute("z", &data.z);

				vData.push_back(data);
			}
		}

		if (numFloats) {
			initScript = pRoot->FirstChildElement("Floats");
			dataElement = initScript->FirstChildElement("float");
			for (int i = 0; i < numFloats; ++i) {
				if (i > 0)
					dataElement = dataElement->NextSiblingElement("float");
				float data;
				dataElement->QueryFloatAttribute("f", &data);
				fData.push_back(data);
			}
		}

		return tinyxml2::XML_SUCCESS;

	};
};