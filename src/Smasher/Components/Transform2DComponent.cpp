#include "Smasher/Base.h"
#include "Smasher/Components/Transform2DComponent.h"

namespace Smasher {
	Transform2DComponent& Transform2DComponent::SetPosition(sf::Vector2f position) {
		setPosition(position);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::SetPosition(float x, float y)
	{
		setPosition(x, y);
		return* this;
	}

	Transform2DComponent& Transform2DComponent::SetRotation(Degrees angle) {
		setRotation((Degrees)angle);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::SetScale(sf::Vector2f factors) {
		setScale(factors);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::SetScale(float x, float y)
	{
		setScale(x, y);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::SetOrigin(float x, float y)
	{
		setOrigin(x, y);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::SetOrigin(sf::Vector2f origin) {
		setOrigin(origin);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::Move(sf::Vector2f offset) {
		move(offset);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::Move(float x, float y) {
		move(x, y);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::Rotate(Degrees angle) {
		rotate((float)angle);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::Scale(sf::Vector2f factor) {
		scale(factor);
		return *this;
	}

	Transform2DComponent& Transform2DComponent::Scale(float x, float y) {
		scale(x, y);
		return *this;
	}
}