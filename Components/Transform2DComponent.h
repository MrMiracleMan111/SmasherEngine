#pragma once
#include <SFML/System.hpp>
#include "Base.h"
#include "Component.h"

namespace Smasher {
	class Transform2DComponent : public Component {
		Transform2DComponent(Smasher::Entity& entity) :
			Smasher::Component(entity) {
		};
		Transform2DComponent(Smasher::Entity& entity, sf::Transform transform) :
			Smasher::Component(entity), m_Transform(transform) {
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