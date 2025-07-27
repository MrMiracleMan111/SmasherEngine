#pragma once
#include <SFML/System.hpp>
#include "Base.h"
#include "IComponent.h"

namespace Smasher {
	class Transform2DComponent : public IComponent {
		Transform2DComponent() = delete;

		Transform2DComponent(Smasher::Entity& entity) :
			Smasher::IComponent(entity) {
		};

		Transform2DComponent(Smasher::Entity& entity, sf::Transform transform) :
			Smasher::IComponent(entity), m_Transform(transform) {
		};
		Transform2DComponent& operator=(Transform2DComponent&&) = default; // So that parent move assignment is called

		sf::Vector2f GetPosition() {
			return sf::Vector2f(m_Transform.getMatrix()[2], m_Transform.getMatrix()[5]);
			//return m_Transform.transformPoint(0, 0);
		}
		float GetRotation();

		void SetPosition(sf::Vector2f position);
		void SetRotation(float angle);

	protected:
		sf::Transform m_Transform;
	};
}