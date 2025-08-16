#include <SFML/Window.hpp>
#include "BallComponent.h"
#include "Components/Transform2DComponent.h"
#include "Components/DrawableComponent.h"
#include "Components/CameraComponent.h"
#include "Entity.h"

BallComponent& BallComponent::SetCamera(Smasher::CameraComponent& rCamera)
{
	m_CameraComponentPtr = &rCamera;
	return *this;
}

void BallComponent::SetEntity(Smasher::Entity& rEntity)
{
	IComponent::SetEntity(rEntity);
	rEntity.DependsOnComponent<Smasher::DrawableComponent>();
	m_DrawableComponentPtr = &rEntity.GetComponent<Smasher::DrawableComponent>();
	m_Collider.left = (int)m_DrawableComponentPtr->GetPosition().x;
	m_Collider.top = (int)m_DrawableComponentPtr->GetPosition().y;
	m_Collider.width = (int)m_DrawableComponentPtr->GetScale().x;
	m_Collider.height = (int)m_DrawableComponentPtr->GetScale().y;
}

void BallComponent::StaticUpdateComponent(BallComponent& self, Smasher::Millisecond delta) {
	sf::Vector2f position = self.m_DrawableComponentPtr->GetPosition();
	sf::Vector2u windowSizeUint = self.GetEntity().GetEngine().GetWindow().getSize();
	sf::Vector2f windowSize((float)windowSizeUint.x, (float)(windowSizeUint.y));

	position += (self.m_Velocity * (float)delta.count() * 0.001f);

	//if (position.x < 0 || position.y < 0 ||
	//	position.x > windowSize.x || position.y > windowSize.y) {
	//	Smasher::Entity& entity = self.GetEntity();
	//	entity.GetGameState().RemoveEntity(entity.GetUUID());
	//	return;
	//}

	if (position.x <= 0) {
		self.m_Velocity.x = -self.m_Velocity.x;
		position.x = 0;

		Smasher::Entity& entity = self.GetEntity();
		//entity.GetGameState().RemoveEntity(entity.GetUUID());
		return;
	}
	if (position.y <= 0) {
		self.m_Velocity.y = -self.m_Velocity.y;
		position.y = 0;

		Smasher::Entity& entity = self.GetEntity();
		//entity.GetGameState().RemoveEntity(entity.GetUUID());
		return;
	}

	if (position.x >= windowSize.x) {
		self.m_Velocity.x = -self.m_Velocity.x;
		position.x = windowSize.x;
	}
	if (position.y >= windowSize.y) {
		self.m_Velocity.y = -self.m_Velocity.y;
		position.y = windowSize.y;
	}

	self.m_DrawableComponentPtr->SetPosition(position);

	if (self.m_CameraComponentPtr) {
		Smasher::Degrees rotation = 10.0f * ((float)delta.count() / 1000.0f);
		self.m_CameraComponentPtr->
			 Rotate(rotation)
			.SetPosition(position)
			.ApplyToTarget(); // Apply changes to view
	}
}

BallComponent& BallComponent::SetVelocity(sf::Vector2f velocity) {
	m_Velocity = velocity;
	return *this;
}
