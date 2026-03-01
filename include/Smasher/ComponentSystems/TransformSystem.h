#pragma once
#include <glm/glm.hpp>
#include <entt/entity/registry.hpp>
#include <SFML/Graphics.hpp>
#include "Smasher/Base.h"
#include "Smasher/ErrorCodes.h"

/**
Right Hand Coordinate System

X - RIGHT
Y - UP
Z - FORWARD

*/
namespace Smasher {
	namespace TransformSystem {
		struct Component {
			glm::vec3 _position;
			glm::vec3 _rotation; // Radians
			glm::vec4 _quatRotation;
			glm::vec3 _scale;

			// Cached, updated upon change
			glm::mat4x4 _globalMatrix;
			bool _dirty;
		};

		SMASHER_API const ErrorCode Initialize(entt::registry& registry) { return ERROR_NoError; };
		SMASHER_API const ErrorCode Teardown(entt::registry& registry) { return ERROR_NoError; };

		SMASHER_API void SetPosition(Component &component, glm::vec3 position);
		SMASHER_API void SetPosition(Component &component, float x, float y, float z);
		SMASHER_API void SetRotation(Component& component, glm::vec3 rotation);
		SMASHER_API void SetRotation(Component &component, Degrees pitch, Degrees roll, Degrees yaw);
		SMASHER_API void SetRotationRad(Component& component, Radians pitch, Radians roll, Radians yaw);
		SMASHER_API void SetScale(Component &component, glm::vec3 factors);
		SMASHER_API void SetScale(Component &component, float x, float y, float z);

		SMASHER_API glm::vec3 GetPosition(Component& component);
		SMASHER_API glm::vec3 GetRotation(Component& component);
		SMASHER_API glm::vec3 GetScale(Component& component);

		SMASHER_API const glm::mat4 &GetTransform(Component& component);
	}
}