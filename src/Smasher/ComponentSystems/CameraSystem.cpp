#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "Smasher/ComponentSystems/CameraSystem.h"
#include "Smasher/ComponentSystems/TransformSystem.h"
#include "Smasher/ErrorCodes.h"



namespace Smasher {
	namespace CameraSystem {
		ErrorCode Initialize(entt::registry& registry) {
			if (registry.ctx().contains<Context>()) {
				return ERROR_SystemAlreadyInitialized;
			}

			registry.ctx().emplace<Context>();

			return ERROR_NoError;
		}

		ErrorCode Teardown(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_NoError;
			}
			return ERROR_NoError;
		}

		// Should be called before TransformSystem::Update
		// Updates view matrices for all cameras
		ErrorCode Update(entt::registry& registry) {
			if (!registry.ctx().contains<Context>()) {
				return ERROR_SystemNotInitialized;
			}

			auto view = registry.view<TransformSystem::Component, Component>();
			for (auto [entity, cameraTransform, cameraComp] : view.each()) {
				if (TransformSystem::HasTransformChanged(cameraTransform)) {
					ComputeViewMatrix(cameraComp, TransformSystem::GetTransform(cameraTransform));
					const glm::mat4& matrix = GetViewMatrix(cameraComp);
					//std::cout << "View Matrix" << glm::to_string(TransformSystem::GetTransform(cameraTransform)) << std::endl;
				}
			}

			return ERROR_NoError;
		}

		Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity) {
			assert(registry.all_of<TransformSystem::Component>(entity) && "StaticMeshSystem::Component requires TransformSystem::Component");

			if (!registry.ctx().contains<Context>()) {
				return Expected<std::reference_wrapper<Component>>::Error(ERROR_SystemNotInitialized);
			}

			Context& ctx = registry.ctx().get<Context>();
			Component& component = registry.emplace<Component>(entity);
			return std::ref(component);
		}


		float GetFOV(const Component& component) {
			return component._fov;
		}

		float GetNearClipPlane(const Component& component) {
			return component._nearClipPlane;
		}

		float GetFarClipPlane(const Component& component) {
			return component._farClipPlane;
		}

		float GetFarAspectRatio(const Component& component) {
			return component._aspectRatio;
		}

		const glm::mat4& GetProjectionMatrix(const Component& component) {
			return component._projectionMatrix;
		}

		const glm::mat4& GetViewMatrix(const Component& component) {
			return component._viewMatrix;
		}


		void SetFOV(Component& component, Degrees fov) {
			component._fov = fov;
		}

		void SetNearClipPlane(Component& component, float nearClipPlane) {
			component._nearClipPlane = nearClipPlane;
		}

		void SetFarClipPlane(Component& component, float farClipPlane) {
			component._farClipPlane = farClipPlane;
		}

		void SetAspectRatio(Component& component, float aspectRatio) {
			component._aspectRatio = aspectRatio;
		}

		// From formula at website
		// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix.html
		// https://johannesugb.github.io/gpu-programming/setting-up-a-proper-vulkan-projection-matrix/
		void ComputeProjectionMatrix(Component& component) {
			constexpr glm::mat4 reverseZ {
				1.f,	0.f,	0.f,	0.f,
				0.f,	1.f,	0.f,	0.f,
				0.f,	0.f,   -1.f,	0.f,
				0.f,	0.f,	1.f,	1.f,
			};
			//component._projectionMatrix = reverseZ * glm::perspective(component._fov, component._aspectRatio, component._nearClipPlane, component._farClipPlane);
			component._projectionMatrix = glm::perspectiveRH_ZO(glm::radians(component._fov), component._aspectRatio, component._nearClipPlane, component._farClipPlane);
			//component._projectionMatrix = reverseZ * glm::infinitePerspectiveRH_ZO(component._fov, component._aspectRatio, component._nearClipPlane);

			//float aspect = component._aspectRatio;
			//float scaleFactor = 1 / (glm::tan(glm::radians(component._fov / 2.f)));
			//float near = component._nearClipPlane;
			//float far = component._farClipPlane;
			//float mult = (far) / (near - far);
			//
			//float fact1 = -(far - near) / (near - far);
			//float fact2 = 2 * (far * near) / (near - far);
			//// Column-Major Order
			//component._projectionMatrix = glm::transpose(glm::mat4(
			//	(1.f / aspect) * scaleFactor,           0.f,       0.f,			  0.f,
			//							 0.f,   scaleFactor,	   0.f,			  0.f,
			//							 0.f,		    0.f,	-fact1,			fact2,
			//							 0.f,		    0.f,	   1.f,			  0.f
			//));
		}

		void ComputeViewMatrix(Component& component, const glm::mat4& transform) {
			component._viewMatrix = glm::inverse(transform);
		}
	}
}