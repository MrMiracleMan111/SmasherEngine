#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entt/entity/registry.hpp>
#include "Smasher/Base.h"
#include "Smasher/ComponentSystems/TransformSystem.h"

namespace Smasher {
	namespace TransformSystem {
		Expected<std::reference_wrapper<Component>> AddComponent(entt::registry& registry, entt::entity entity) {
			Component& component = registry.emplace<Component>(entity);
			return std::ref(component);
		}

		void SetPosition(Component& component, glm::vec3 position) {
			component._position = position;
			component._dirty = true;
			component._hasChanged = true;
		}

		void SetPosition(Component& component, float x, float y, float z) {
			component._position = glm::vec3(x, y, z);
			component._dirty = true;
			component._hasChanged = true;
		}

		// Expects radians
		void SetEuler(Component& component, glm::vec3 rotation) {
			component._rotation = glm::quat(rotation);
			component._dirty = true;
			component._hasChanged = true;
		}

		void SetEuler(Component& component, Radians pitch, Radians roll, Radians yaw) {
			component._rotation = glm::quat(glm::vec3(pitch, roll, yaw));
			component._dirty = true;
			component._hasChanged = true;
		}

		void SetEulerDeg(Component& component, Degrees pitch, Degrees roll, Degrees yaw) {
			glm::vec3 euler = glm::radians(glm::vec3(pitch, roll, yaw));
			component._rotation = glm::quat(euler);
			component._dirty = true;
			component._hasChanged = true;
		}

		void SetRotation(Component& component, glm::quat quaternion) {
			component._rotation = quaternion;
			component._dirty = true;
			component._hasChanged = true;
		}


		void SetScale(Component& component, glm::vec3 scale) {
			component._scale = scale;
			component._dirty = true;
			component._hasChanged = true;
		}

		void SetScale(Component& component, float x, float y, float z) {
			component._scale = glm::vec3(x, y, z);
			component._dirty = true;
			component._hasChanged = true;
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

			return glm::mat3 {
				component._scale.x * cosine,	component._scale.x * sine,		0.f,
				component._scale.y * sine,		component._scale.y * cosine,	0.f,
				component._position.x,			component._position.y,			1.f
			};
		}

		// Updates global matrix of transform
		// using component's position, rotation, scale
		void ComputeGlobalMatrix(Component& component) {
			component._globalMatrix =
				glm::translate(glm::mat4(1.f), component._position) *
				glm::scale(glm::mat4(1.f), component._scale) * glm::mat4(1.f) *
				glm::toMat4(component._rotation);
			component._dirty = true;
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

		bool HasChanged(Component& component) {
			return component._hasChanged;
		}

		ErrorCode Initialize(entt::registry& registry) {
			return ERROR_NoError;
		}
		ErrorCode Teardown(entt::registry& registry) {
			return ERROR_NoError;
		}

		ErrorCode Update(entt::registry& registry) {
			auto view = registry.view<Component>();
			for (auto [entity, transform] : view.each()) {
				transform._hasChanged = false;
			}
			return ERROR_NoError;
		}
	}
}