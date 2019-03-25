#pragma once
#include "../Utility/componentIncludes.h"
#include "Systems/characterController.h"
#include <array>

enum class LightState
{
	Static,
	Moving
};

class LightController {
private:
	struct {
		TransformComponent* transform;
		NodeComponent* node;
		LightState state = LightState::Static;
	} light;

	struct {
		TransformComponent* transform;
		CharacterState* state;
	} character;
	
	glm::vec3 lightPositions[3];
	float levelHeights[3];
	int32_t levelIndex = 0;
	float lightSpeed = .01f;
	TransformSystem* ts;

public:
	LightController() {
		lightPositions[0] = glm::vec3(-13.75f, 14.1f, 9.91f);
		levelHeights[0] = 4.5f;
		
		lightPositions[1] = glm::vec3(17.352f, 44.544f, 0.5f);
		levelHeights[1] = 24.f;

		lightPositions[2] = glm::vec3(62.582f, 14.094f, 8.188f);
		levelHeights[2] = 35.f;
	};
	~LightController() {};

	void initialize(TransformComponent* tc_light, NodeComponent* nc, TransformComponent* tc_char, CharacterState* cs, TransformSystem* ts) {
		light.transform = tc_light;
		light.node = nc;
		character.transform = tc_char;
		character.state = cs;
		this->ts = ts;
	};

	void updateLight(float dt) {
		checkLight();
		if (light.state == LightState::Moving) {
			light.transform->local.position += moveTowards(light.transform->local.position, lightPositions[levelIndex] * 1.01f) * dt;
			light.transform->local.position = glm::clamp(light.transform->local.position, glm::vec3(0.f), lightPositions[levelIndex]);
			//light.component->pos = light.transform->global.position;
			//rs->updateLight(light.component);
			ts->SQTTransform(light.node, sqt());
			if (light.transform->local.position == lightPositions[levelIndex])
				light.state = LightState::Static;
		}
	}

	void checkLight() {
		if (light.state == LightState::Static) {
			if (character.transform->global.position.y > levelHeights[levelIndex]) {
				if (*character.state == CharacterState::GROUNDED) {
					light.state = LightState::Moving;
					levelIndex++;
					//light.component->pos = light.transform->global.position;
					//rs->updateLight(light.component);
				}
			}
		}
	}

	glm::vec3 moveTowards(glm::vec3 from, glm::vec3 to) {
		glm::vec3 diff = from - to;
		return diff *= lightSpeed;
	}
};