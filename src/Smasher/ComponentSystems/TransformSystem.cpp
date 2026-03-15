#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entt/entity/registry.hpp>
#include "Smasher/Base.h"
#include "Smasher/ComponentSystems/TransformSystem.h"

namespace Smasher {
	namespace TransformSystem {
		void SetPosition(Component& component, glm::vec3 position) {
		
		}

		void SetPosition(Component& component, float x, float y, float z) {
		
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

		void SetScale(Component& component, glm::vec3 factors) {
		
		}

		void SetScale(Component& component, float x, float y, float z) {
		
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

		// Updates global matrix of transform
		// using component's position, rotation, scale
		void ComputeGlobalMatrix(Component& component) {
			float angle = 0.f;
			component._globalMatrix = glm::mat4(); // Identity matrix
			glm::scale(component._globalMatrix, component._scale);
			glm::rotate(component._globalMatrix, angle, glm::vec3(0.f, 0.f, 1.f));
			glm::translate(component._globalMatrix, component._position);
		}

		const glm::mat4& GetTransform(Component& component) {
			if (!component._dirty) {
				return component._globalMatrix;
			}

			// Compute Global matrix
		}
	}
}