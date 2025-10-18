#pragma once
#include <chrono>
#include "Base.h"
#include "IComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"

namespace Smasher {
	class Transform2DComponent;
	class DrawableComponent;
	class CameraComponent;
}

class BallComponent : public Smasher::IComponent {
public:
	BallComponent() = default;
	BallComponent(sf::Vector2f velocity) : m_Velocity(velocity) {}

	BallComponent& SetCamera(Smasher::CameraComponent& rCamera);

	void SetEntity(Smasher::Entity& rEntity) override;

	static void StaticUpdateComponent(BallComponent& self, Smasher::Millisecond delta);

	BallComponent& SetVelocity(sf::Vector2f velocity);

	const sf::Vector2f& GetVelocity() const { return m_Velocity; }

private:
	sf::Vector2f m_Velocity;
	sf::IntRect m_Collider;
	Smasher::DrawableComponent* m_DrawableComponentPtr = nullptr;
	Smasher::CameraComponent* m_CameraComponentPtr = nullptr;
};
