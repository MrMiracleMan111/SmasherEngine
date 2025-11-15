#include "Smasher/AABBPhysics.h"

namespace Smasher {
	AABBPhysicsComponent::AABBPhysicsComponent(b2DynamicTree& BVH, int b2TreeProxyId) :
		IComponent(),
		m_b2BVHRef(BVH),
		m_b2TreeProxyId(b2TreeProxyId),
		m_OldPosition(0, 0),
		m_Position(0, 0),
		m_Velocity(0, 0),
		m_Acceleration(0, 0),
		m_Static(false)
	{
		b2AABB tmp = AABBPhysicsComponent::DEFAULT_AABB;
		b2Vec2 halfScale = tmp.upperBound - tmp.lowerBound;
		m_Scale = halfScale;
		halfScale *= 0.5f;
		m_Position = tmp.lowerBound + halfScale;
		m_OldPosition = m_Position;
	}

	AABBPhysicsComponent::~AABBPhysicsComponent() {
		if (m_b2TreeProxyId != -1) {
			b2DynamicTree_DestroyProxy(&m_b2BVHRef, m_b2TreeProxyId);
		}
	}

	AABBPhysicsComponent& AABBPhysicsComponent::MakeStatic() {
		m_Static = true;
		return *this;
	}

	AABBPhysicsComponent& AABBPhysicsComponent::MakeDynamic() {
		m_Static = false;
		return *this;
	}

	AABBPhysicsComponent& AABBPhysicsComponent::Move(sf::Vector2f delta) {
		m_Position.x += delta.x;
		m_Position.y += delta.y;
		b2DynamicTree_MoveProxy(&m_b2BVHRef, m_b2TreeProxyId, Getb2AABB());
		m_PhysicsChanged = true;
		return *this;
	}

	AABBPhysicsComponent& AABBPhysicsComponent::MoveTo(sf::Vector2f position) {
		m_Position.x = position.x;
		m_Position.y = position.y;
		b2DynamicTree_MoveProxy(&m_b2BVHRef, m_b2TreeProxyId, Getb2AABB());
		m_PhysicsChanged = true;
		return *this;
	}

	AABBPhysicsComponent& AABBPhysicsComponent::SetPosition(sf::Vector2f position) {
		m_PhysicsChanged = false;

		m_Position.x = position.x;
		m_Position.y = position.y;
		b2DynamicTree_MoveProxy(&m_b2BVHRef, m_b2TreeProxyId, Getb2AABB());

		Setb2OldPosition(m_Position);
		return *this;
	}

	AABBPhysicsComponent& AABBPhysicsComponent::SetScale(sf::Vector2f scale) {
		m_Scale.x = scale.x;
		m_Scale.y = scale.y;
		b2DynamicTree_MoveProxy(&m_b2BVHRef, m_b2TreeProxyId, Getb2AABB());
		//b2DynamicTree_EnlargeProxy(&m_b2BVHRef, m_b2TreeProxyId, Getb2AABB());
		return *this;
	}

	AABBPhysicsComponent& AABBPhysicsComponent::SetAcceleration(sf::Vector2f acceleration) {
		m_Acceleration.x = acceleration.x;
		m_Acceleration.y = acceleration.y;
		return *this;
	}

	AABBPhysicsComponent& AABBPhysicsComponent::SetVelocity(sf::Vector2f velocity) {
		m_Velocity.x = velocity.x;
		m_Velocity.y = velocity.y;
		return *this;
	}

	AABBPhysicsComponent& AABBPhysicsComponent::SetOnCollisionCallback(std::function<void(AABBPhysicsComponent&)> callback) {
		m_OnCollisionCallback = callback;
		return *this;
	}


	sf::Vector2f AABBPhysicsComponent::GetPosition() const {
		return sf::Vector2f{ m_Position.x, m_Position.y };
	};


	sf::Vector2f AABBPhysicsComponent::GetOldPosition() const {
		return sf::Vector2f(m_OldPosition.x, m_OldPosition.y);
	}

	sf::Vector2f AABBPhysicsComponent::GetScale() const {
		return sf::Vector2f{ m_Scale.x, m_Scale.y };
	}

	sf::Vector2f AABBPhysicsComponent::GetVelocity() const {
		return sf::Vector2f(m_Velocity.x, m_Velocity.y);
	}
	sf::Vector2f AABBPhysicsComponent::GetAcceleration() const {
		return sf::Vector2f(m_Acceleration.x, m_Acceleration.y);
	}

	b2AABB AABBPhysicsComponent::Getb2AABB() const {
		b2Vec2 pos{ m_Position.x, m_Position.y };
		b2AABB aabb;
		b2Vec2 halfScale = m_Scale;
		halfScale *= 0.5f;
		aabb.lowerBound = pos - halfScale;
		aabb.upperBound = pos + halfScale;
		return aabb;
	}

	void AABBPhysicsComponent::OnCollide(AABBPhysicsComponent& other) {
		if (!m_OnCollisionCallback) {
			return;
		}

		m_OnCollisionCallback(other);
	}

}