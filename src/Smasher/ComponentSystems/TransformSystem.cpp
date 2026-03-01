#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Smasher/Base.h"
#include "Smasher/ComponentSystems/TransformSystem.h"

namespace Smasher {
	namespace TransformSystem {
		void SetPosition(Component& component, glm::vec3 position) {
		
		}

		void SetPosition(Component& component, float x, float y, float z) {
		
		}

		void SetRotation(Component& component, glm::vec3 rotation) {
		
		}

		void SetRotation(Component& component, Degrees pitch, Degrees roll, Degrees yaw) {
		
		}

		void SetRotationRad(Component& component, Radians pitch, Radians roll, Radians yaw) {

		}

		void SetScale(Component& component, glm::vec3 factors) {
		
		}

		void SetScale(Component& component, float x, float y, float z) {
		
		}


		glm::vec3 GetPosition(Component& component) {
			return component._position;
		}

		glm::vec3 GetRotation(Component& component) {
			return component._rotation;
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