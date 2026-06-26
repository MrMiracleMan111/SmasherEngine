#include "Smasher/Base.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <entt/entt.hpp>
#include "Smasher/ComponentSystems/TransformSystem.h"

using namespace entt::literals;

namespace Smasher {
	namespace TransformSystem {
		Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity) {
			Component& component = registry.emplace<Component>(entity);
			return std::ref(component);
		}

		void SetPosition(Component &component, glm::vec3 position) {
				component._position = position;
				component._dirty = true;
				component._hasChanged = true;
		}

		void SetPosition(Component &component, float x, float y, float z) {
				component._position = glm::vec3(x, y, z);
				component._dirty = true;
				component._hasChanged = true;
		}

		// Expects radians
		void SetEuler(Component &component, glm::vec3 rotation) {
				component._rotation = glm::quat(rotation);
				component._dirty = true;
				component._hasChanged = true;
		}

		void SetEuler(Component &component, Radians pitch, Radians yaw, Radians roll) {
				component._rotation = glm::quat(glm::vec3(pitch, yaw, roll));
				component._dirty = true;
				component._hasChanged = true;
		}

		void SetEulerDeg(Component &component, Degrees pitch, Degrees yaw, Degrees roll) {
				glm::vec3 rads = glm::radians(glm::vec3(pitch, yaw, roll));
				component._rotation = glm::quat(rads);
				component._dirty = true;
				component._hasChanged = true;
		}

		void SetRotation(Component &component, glm::quat quaternion) {
				component._rotation = quaternion;
				component._dirty = true;
				component._hasChanged = true;
		}


		void SetScale(Component &component, glm::vec3 scale) {
				component._scale = scale;
				component._dirty = true;
				component._hasChanged = true;
		}

		void SetScale(Component &component, float x, float y, float z) {
				component._scale = glm::vec3(x, y, z);
				component._dirty = true;
				component._hasChanged = true;
		}

		void Rotate(Component& component, glm::quat quaternion, bool local) {
			if (local) {
				component._rotation = component._rotation * quaternion;
			}
			else {
				component._rotation = quaternion * component._rotation;
			}
			component._dirty = true;
			component._hasChanged = true;
		}

		void RotateEuler(Component& component, Radians pitch, Radians yaw, Radians roll, bool local) {
			glm::quat p = glm::angleAxis(pitch, glm::vec3(1.f, 0.f, 0.f));
			glm::quat y = glm::angleAxis(yaw,	glm::vec3(0.f, 1.f, 0.f));
			glm::quat r = glm::angleAxis(roll,	glm::vec3(0.f, 0.f, 1.f));

			glm::quat delta = p * y * r;

			if (local) {
				component._rotation = component._rotation * delta;
			}
			else {
				component._rotation = delta * component._rotation;
			}
			component._dirty = true;
			component._hasChanged = true;
		}

		void RotateEulerDeg(Component& component, Degrees pitch, Degrees yaw, Degrees roll, bool local) {
			RotateEuler(component, glm::radians(pitch), glm::radians(yaw), glm::radians(roll), local);
		}

		glm::vec3 GetPosition(Component& component) {
			return component._position;
		}

		glm::quat GetRotation(Component& component) {
			return component._rotation;
		}

		glm::vec3 GetEuler(Component& component) {
			return glm::eulerAngles(component._rotation);
		}

		glm::vec3 GetEulerDeg(Component& component) {
			glm::vec3 euler = glm::eulerAngles(component._rotation);
			return glm::degrees(euler);
		}

		glm::vec3 GetScale(Component& component) {
			return component._scale;
		}

		glm::mat3 Compute2DTransform(Component& component) {
			// Extract Z-Axis rotation from rotation
			float angle = std::atan2(2.0f * (component._rotation.w * component._rotation.z + component._rotation.x * component._rotation.y),
				1.0f - 2.0f * (component._rotation.y * component._rotation.y + component._rotation.z * component._rotation.z));
			float cosine = std::cos(angle);
			float sine = std::sin(angle);

			// Return Column-Major ordered matrix
			return glm::transpose(glm::mat3 {
				component._scale.x * cosine,	component._scale.x * sine,		0.f,
				component._scale.y * sine,		component._scale.y * cosine,	0.f,
				component._position.x,			component._position.y,			1.f
			});
		}

		// Updates global matrix of transform
		// using component's position, rotation, scale
		void ComputeGlobalMatrix(Component& component) {
			component._globalMatrix =
				glm::translate(glm::mat4(1.f), component._position) *
				glm::scale(glm::mat4(1.f), component._scale) *
				glm::mat4(1.f) * glm::toMat4(component._rotation);
			component._dirty = false;
		}

		const glm::mat4& GetTransform(Component& component) {
			if (!component._dirty) {
				return component._globalMatrix;
			}

			// Compute Global matrix
			ComputeGlobalMatrix(component);
			return component._globalMatrix;
		}

		glm::mat3 Extract2DTransform(const glm::mat4& matrix) {
			// [column][row]
			// column major
			return glm::mat3 {
				matrix[0][0], matrix[0][1], 0,
				matrix[1][0], matrix[1][1], 0,
				matrix[3][0], matrix[3][1], 1
			};
		}

		bool HasTransformChanged(Component& component) {
			return component._hasChanged;
		}

		ErrorCode Initialize(entt::registry& registry) {
			// Initialize Dirty Components storage
			registry.storage<entt::reactive>("TransformComponentChange"_hs);
			return ERROR_NoError;
		}

		// Mark TransformComponent of entity as modified
		// Should be called AFTER all components for entity are initialized
		ErrorCode MarkDirty(entt::registry& registry, entt::entity entity) {
			registry.patch<Component>(entity);
			return ERROR_NoError;
		}

		// Container housing modified transform components
		entt::registry::storage_for_type<entt::reactive>& GetDirty(entt::registry& registry) {
			return registry.storage<entt::reactive>("DirtyTransformComponents"_hs);
		}

		// Empties the container housing modified transform components
		ErrorCode ClearDirty(entt::registry& registry) {
			registry.storage<entt::reactive>("DirtyTransformComponents"_hs).clear();
			return ERROR_NoError;
		}

		ErrorCode Teardown(entt::registry& registry) {
			return ERROR_NoError;
		}
	}
}