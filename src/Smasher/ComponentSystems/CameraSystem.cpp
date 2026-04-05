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
			component._projectionMatrix = glm::perspective(component._fov, component._aspectRatio, component._nearClipPlane, component._farClipPlane);

			//float aspect = component._aspectRatio;
			//float scaleFactor = 1 / (glm::tan(glm::radians(component._fov / 2.f)));
			//float near = component._nearClipPlane;
			//float far = component._farClipPlane;
			//float mult = (component._farClipPlane) / (far - near);
			//
			//float fact1 = -(far + near) / (far - near);
			//float fact2 = -2 * (far * near) / (far - near);
			//component._projectionMatrix = glm::mat4(
			//scaleFactor * (1.f/aspect),         0.f,       0.f,			  0.f,
			//	                   0.f, scaleFactor,	   0.f,			  0.f,
			//	                   0.f,		    0.f,	  mult,	   -near*mult,
			//	                   0.f,		    0.f,	   1.f,			  0.f
			//);
		}

		void ComputeViewMatrix(Component& component, const glm::mat4& transform) {
			component._viewMatrix = glm::inverse(transform);
		}
	}
}