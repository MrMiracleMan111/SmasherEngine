#pragma once
#include "Base.h"
#include "IComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"

namespace Smasher {
	class Transform2DComponent;
	class DrawableComponent;
}

class BallComponent : public Smasher::IComponent {
public:
	BallComponent() = default;
	BallComponent(sf::Vector2f velocity) : m_Velocity(velocity) {}

	void SetEntity(Smasher::Entity& rEntity) override;

	static void StaticUpdateComponent(BallComponent& self, Smasher::Millisecond delta);

	BallComponent& SetVelocity(sf::Vector2f velocity);

	const BallComponent& GetVelocity() const;

private:
	sf::Vector2f m_Velocity;
	sf::IntRect m_Collider;
	Smasher::Transform2DComponent* m_TransformComponentPtr = nullptr;
	Smasher::DrawableComponent* m_DrawableComponentPtr = nullptr;

};