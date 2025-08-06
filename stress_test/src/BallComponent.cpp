#include "BallComponent.h"
#include "Components/Transform2DComponent.h"
#include "Components/DrawableComponent.h"
#include "Entity.h"

void BallComponent::SetEntity(Smasher::Entity& rEntity)
{
	IComponent::SetEntity(rEntity);
	assert(rEntity.HasComponent<Smasher::Transform2DComponent>());
	m_TransformComponentPtr = &rEntity.GetComponent<Smasher::Transform2DComponent>();
	assert(rEntity.HasComponent<Smasher::DrawableComponent>());
	m_DrawableComponentPtr = &rEntity.GetComponent<Smasher::DrawableComponent>();
	m_Collider.left = (int)m_TransformComponentPtr->GetPosition().x;
	m_Collider.top = (int)m_TransformComponentPtr->GetPosition().y;
	m_Collider.width = (int)m_TransformComponentPtr->GetScale().x;
	m_Collider.height = (int)m_TransformComponentPtr->GetScale().y;
}

void BallComponent::StaticUpdateComponent(BallComponent& self, Smasher::Millisecond delta) {
	sf::Vector2f position = self.m_TransformComponentPtr->GetPosition();

	sf::Vector2u windowSizeUint = self.GetEntity().GetEngine().GetWindow().getSize();
	sf::Vector2f windowSize((float)windowSizeUint.x, (float)(windowSizeUint.y));

	position += (self.m_Velocity * (float)delta.count() * 0.001f);

	if (position.x < 0) {
		self.m_Velocity.x = -self.m_Velocity.x;
		position.x = 0;
	}
	if (position.y < 0) {
		self.m_Velocity.y = -self.m_Velocity.y;
		position.y = 0;
	}

	if (position.x > windowSize.x) {
		self.m_Velocity.x = -self.m_Velocity.x;
		position.x = windowSize.x;
	}
	if (position.y > windowSize.y) {
		self.m_Velocity.y = -self.m_Velocity.y;
		position.y = windowSize.y;
	}

	self.m_TransformComponentPtr->SetPosition(position);
	self.m_DrawableComponentPtr->PushToGPU();
}

BallComponent& BallComponent::SetVelocity(sf::Vector2f velocity) {
	m_Velocity = velocity;
	return *this;
}

const sf::Vector2f& BallComponent::GetVelocity() const {
	return m_Velocity;
}
