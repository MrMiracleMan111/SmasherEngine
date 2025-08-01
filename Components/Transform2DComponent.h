#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "IComponent.h"

namespace Smasher {
	class SMASHER_API Transform2DComponent : public IComponent, private sf::Transformable {
	public:
		Transform2DComponent& SetPosition(sf::Vector2f position) {
			setPosition(position);
			return *this;
		}

		Transform2DComponent& SetRotation(Degrees angle) {
			setRotation((Degrees)angle);
			return *this;
		}

		Transform2DComponent& SetScale(sf::Vector2f factors) {
			setScale(factors);
			return *this;
		}

		Transform2DComponent& SetOrigin(sf::Vector2f origin) {
			setOrigin(origin);
			return *this;
		}

		sf::Vector2f GetPosition() const {
			return getPosition();
		}

		Degrees GetRotation() const {
			return (Degrees)(getRotation());
		}

		sf::Vector2f GetScale() const {
			return getScale();
		}

		sf::Vector2f GetOrigin() const {
			return getOrigin();
		}

		Transform2DComponent& Move(sf::Vector2f offset) {
			move(offset);
			return *this;
		}

		Transform2DComponent& Rotate(Degrees angle) {
			rotate((float)angle);
			return *this;
		}

		Transform2DComponent& Scale(sf::Vector2f factor) {
			scale(factor);
			return *this;
		}

		const sf::Transform & GetTransform() const {
			return getTransform();
		}

		const sf::Transform & GetInverseTransform() const {
			return getInverseTransform();
		}
	};
}