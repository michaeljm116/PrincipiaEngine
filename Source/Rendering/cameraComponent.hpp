#pragma once
#include <Artemis/Component.h>
#include <glm/glm.hpp>

namespace Principia {
	struct CameraComponent : public artemis::Component {
		float aspectRatio = 0.f;
		float fov = 0.f;
		glm::mat4 rotM = glm::mat4();


		CameraComponent() {};
		CameraComponent(float ar, float f) :aspectRatio(ar), fov(f) {};
	};

	class Camera
	{
	public:
		float fov = 0;
	private:
		float znear = 0, zfar = 1000;

		void updateViewMatrix()
		{
			glm::mat4 rotM = glm::mat4(1.0f);
			glm::mat4 transM = glm::mat4();

			rotM = glm::rotate(rotM, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			transM = glm::translate(glm::mat4(1.0f), position);

			if (type == CameraType::firstperson)
			{
				matrices.view = rotM * transM;
			}
			else
			{
				matrices.view = transM * rotM;
			}
		};
	public:
		enum CameraType { lookat, firstperson };
		CameraType type = CameraType::lookat;

		glm::vec3 rotation = glm::vec3();
		glm::vec3 position = glm::vec3();

		float rotationSpeed = 1.0f;
		float movementSpeed = 1.0f;

		float aspect = 0.f;

		struct
		{
			glm::mat4 perspective = glm::mat4();
			glm::mat4 view = glm::mat4();
		} matrices;

		bool moving()
		{
			//return INPUT.up || INPUT.right || INPUT.up || INPUT.down;
			return false;
		}

		void setPerspective(float fov, float aspect, float znear, float zfar)
		{
			this->fov = fov;
			this->znear = znear;
			this->zfar = zfar;
			this->aspect = aspect;
			matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
		};

		void updateAspectRatio(float aspect)
		{
			this->aspect = aspect;
			matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
		}

		void setPosition(glm::vec3 position)
		{
			this->position = position;
			updateViewMatrix();
		}

		void setRotation(glm::vec3 rotation)
		{
			this->rotation = rotation;
			updateViewMatrix();
		};

		void rotate(glm::vec3 delta)
		{
			this->rotation += delta;
			updateViewMatrix();
		}

		void setTranslation(glm::vec3 translation)
		{
			this->position = translation;
			updateViewMatrix();
		};

		void translate(glm::vec3 delta)
		{
			this->position += delta;
			updateViewMatrix();
		}

		void update(float deltaTime)
		{
			if (type == CameraType::firstperson)
			{
				if (moving())
				{
					glm::vec3 camFront;
					camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
					camFront.y = sin(glm::radians(rotation.x));
					camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
					camFront = glm::normalize(camFront);

					float moveSpeed = deltaTime * movementSpeed;
					/*
					if (INPUT.up)
						position += camFront * moveSpeed;
					if (INPUT.down)
						position -= camFront * moveSpeed;
					if (INPUT.left)
						position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
					if (INPUT.right)
						position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
					*/
					updateViewMatrix();
				}
			}
		};

		// Update camera passing separate axis data (gamepad)
		// Returns true if view or position has been changed
		bool updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
		{
			bool retVal = false;

			if (type == CameraType::firstperson)
			{
				// Use the common console thumbstick layout		
				// Left = view, right = move

				const float deadZone = 0.0015f;
				const float range = 1.0f - deadZone;

				glm::vec3 camFront;
				camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
				camFront.y = sin(glm::radians(rotation.x));
				camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
				camFront = glm::normalize(camFront);

				float moveSpeed = deltaTime * movementSpeed * 2.0f;
				float rotSpeed = deltaTime * rotationSpeed * 50.0f;

				// Move
				if (fabsf(axisLeft.y) > deadZone)
				{
					float pos = (fabsf(axisLeft.y) - deadZone) / range;
					position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
					retVal = true;
				}
				if (fabsf(axisLeft.x) > deadZone)
				{
					float pos = (fabsf(axisLeft.x) - deadZone) / range;
					position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
					retVal = true;
				}

				// Rotate
				if (fabsf(axisRight.x) > deadZone)
				{
					float pos = (fabsf(axisRight.x) - deadZone) / range;
					rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
					retVal = true;
				}
				if (fabsf(axisRight.y) > deadZone)
				{
					float pos = (fabsf(axisRight.y) - deadZone) / range;
					rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
					retVal = true;
				}
			}
			else
			{
				// todo: move code from example base class for look-at
			}

			if (retVal)
			{
				updateViewMatrix();
			}

			return retVal;
		}

	};
}