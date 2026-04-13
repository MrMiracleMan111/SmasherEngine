#pragma once
#include <glm/glm.hpp>
#include "entt/entity/registry.hpp"
#include "Smasher/Base.h"

namespace Smasher {
	namespace CameraSystem {
		struct SMASHER_API Context {
		};

		struct SMASHER_API Component {
			float _nearClipPlane;
			float _farClipPlane;
			float _aspectRatio;
			Degrees _fov;
			glm::mat4 _projectionMatrix;
			glm::mat4 _viewMatrix;
		};

		SMASHER_API ErrorCode Initialize(entt::registry& registry);
		SMASHER_API ErrorCode Teardown(entt::registry& registry);
		SMASHER_API	ErrorCode Update(entt::registry& registry);

		SMASHER_API Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity);

		SMASHER_API	float GetFOV(const Component& component);
		SMASHER_API	float GetNearClipPlane(const Component& component);
		SMASHER_API	float GetFarClipPlane(const Component& component);
		SMASHER_API	float GetFarAspectRatio(const Component& component);
		SMASHER_API	const glm::mat4& GetProjectionMatrix(const Component& component);
		SMASHER_API	const glm::mat4& GetViewMatrix(const Component& component);

		SMASHER_API	void SetFOV(Component& component, Degrees fov);
		SMASHER_API	void SetNearClipPlane(Component& component, float nearClipPlane);
		SMASHER_API	void SetFarClipPlane(Component& component, float farClipPlane);
		SMASHER_API	void SetAspectRatio(Component& component, float aspect);

		SMASHER_API	void ComputeProjectionMatrix(Component& component);
		SMASHER_API	void ComputeViewMatrix(Component& component, const glm::mat4 &transform);
	}
}