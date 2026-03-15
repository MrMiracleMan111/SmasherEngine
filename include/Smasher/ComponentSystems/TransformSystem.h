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
		// Contains transform information (position, rotation, etc.)
		// members should not directly be accessed, use the Set/Get
		// methods instead.
		struct SMASHER_API Component {
			glm::vec3 _position;
			glm::vec3 _rotation; // Radians
			glm::vec4 _quatRotation;
			glm::vec3 _scale;

			// Cached, updated upon change
			glm::mat4x4 _globalMatrix;
			bool _dirty;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API ErrorCode Update(entt::registry& registry);

		SMASHER_API Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity);

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