#pragma once
#include <SFML/Graphics/Transformable.hpp>
#include "Smasher/Base.h"
#include ""Smasher/IComponent.h""

namespace Smasher {
	class SMASHER_API Transform2DComponent : public IComponent, private sf::Transformable {
	public:
		Transform2DComponent() : IComponent(), sf::Transformable() {
			SetScale(sf::Vector2f(100.0f, 100.0f));
		}

		Transform2DComponent& SetPosition(sf::Vector2f position);

		Transform2DComponent& SetPosition(float x, float y);

		Transform2DComponent& SetRotation(Degrees angle);

		Transform2DComponent& SetScale(sf::Vector2f factors);

		Transform2DComponent& SetScale(float x, float y);

		Transform2DComponent& SetOrigin(sf::Vector2f origin);

		Transform2DComponent& SetOrigin(float x, float y);

		sf::Vector2f GetPosition() const { return getPosition(); }

		Degrees GetRotation() const { return (Degrees)(getRotation()); }

		sf::Vector2f GetScale() const { return getScale(); }

		sf::Vector2f GetOrigin() const { return getOrigin(); }

		const sf::Transform& GetTransform() const { return getTransform(); }

		const sf::Transform& GetInverseTransform() const { return getInverseTransform(); }

		Transform2DComponent& Move(sf::Vector2f offset);

		Transform2DComponent& Move(float x, float y);

		Transform2DComponent& Rotate(Degrees angle);

		Transform2DComponent& Scale(sf::Vector2f factor);

		Transform2DComponent& Scale(float x, float y);
	};
}