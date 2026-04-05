#pragma once
#include <bitset>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
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
			glm::vec3 _position = glm::vec3{0.f, 0.f, 0.f};
			glm::quat _rotation = glm::quat{ 0.f, 0.f, 0.f, 0.f };
			glm::vec3 _scale = glm::vec3{ 1.f, 1.f, 1.f };
			glm::mat4x4 _globalMatrix; // Cached, updated upon change
			bool _dirty; // Indicates that _globalMatrix needs to be updated
			bool _hasChanged; // Has the transform changed this frame
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API ErrorCode Update(entt::registry& registry);

		SMASHER_API Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity);

		SMASHER_API void SetPosition(Component &component, glm::vec3 position);
		SMASHER_API void SetPosition(Component &component, float x, float y, float z);
		// In Radians
		SMASHER_API void SetRotation(Component& component, glm::quat quaternion);
		// In Radians
		SMASHER_API void SetEuler(Component& component, Radians pitch, Radians roll, Radians yaw);
		// In Degrees
		SMASHER_API void SetEulerDeg(Component &component, Degrees pitch, Degrees roll, Degrees yaw);
		// Quaternion
		SMASHER_API void SetScale(Component &component, glm::vec3 scale);
		SMASHER_API void SetScale(Component &component, float x, float y, float z);

		SMASHER_API glm::vec3 GetPosition(Component& component);
		SMASHER_API glm::quat GetRotation(Component& component);

		// In Radians
		SMASHER_API glm::vec3 GetEuler(Component& component);
		// In Degrees
		SMASHER_API glm::vec3 GetEulerDeg(Component& component);
		SMASHER_API glm::vec3 GetScale(Component& component);

		SMASHER_API const glm::mat4 &GetTransform(Component& component);

		// Creates XY Transform from XYZ Transform Matrix
		SMASHER_API glm::mat3 Extract2DTransform(const glm::mat4& matrix);
		SMASHER_API glm::mat3 Compute2DTransform(Component& component);



		SMASHER_API bool HasChanged(Component& component);
	}
}