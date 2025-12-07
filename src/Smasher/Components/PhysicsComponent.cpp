#include <cassert> // Required for assert
#include "box2d/box2d.h"
#include "Smasher/Physics.h"

namespace Smasher {
	PhysicsComponent::PhysicsComponent() {

	}

	PhysicsComponent::~PhysicsComponent() {
		if (m_BodyValid) {
			b2DestroyBody(m_BodyId);
			m_ShapeValid = false;
		}
	}

	PhysicsComponent::PhysicsComponent(PhysicsComponent &&other) noexcept :
		m_BodyId(other.m_BodyId),
		m_BodyValid(other.m_BodyValid),
		m_ShapeId(other.m_ShapeId),
		m_ShapeValid(other.m_ShapeValid)
	{
		other.m_BodyId = b2BodyId{ 0 }; // Nullify other body id
		other.m_BodyValid = false;
		other.m_ShapeId = b2ShapeId{ 0 }; // Nullify other shape id
		other.m_ShapeValid = false;
	}

	PhysicsComponent& PhysicsComponent::operator = (PhysicsComponent &&other) noexcept {
		if (this != &other) {
			if (m_BodyValid) {
				b2DestroyBody(m_BodyId);
				m_ShapeValid = false;
			}
			m_BodyId = other.m_BodyId;
			m_BodyValid = other.m_BodyValid;
			m_ShapeId = other.m_ShapeId;
			other.m_BodyId = b2BodyId{ 0 }; // Nullify other body id
			other.m_BodyValid = false;
			other.m_ShapeId = b2ShapeId{ 0 }; // Nullify other shape id
			other.m_ShapeValid = false;
		}
		return *this;
	}


	void PhysicsComponent::OnAddComponent() {
		const b2WorldId& worldId = GetEntity().GetEngine().GetPhysicsManager().GetWorldId();
		b2BodyDef bodyDef = b2DefaultBodyDef();
		m_BodyId = b2CreateBody(worldId, &bodyDef);
		m_BodyValid = true;
		SetPhysicsType(m_PhysicsType);
	}

	PhysicsComponent& PhysicsComponent::UseRectCollider(float width, float height) {
		if (!m_BodyValid) {
			throw Exceptions::Box2DBodyIdInvalid("The m_BodyId Box2D Body Id is invalid");
		}

		if (m_ShapeValid) {
			// Replace old shape with new one
			b2DestroyShape(m_ShapeId, false);
		}
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		b2Polygon shape = b2MakeBox(width / 2.f, height / 2.f);
		m_ShapeId = b2CreatePolygonShape(m_BodyId, &shapeDef, &shape);
		m_ShapeValid = true;
		return *this;
	}

	PhysicsComponent& PhysicsComponent::UseCircleCollider(float radius) {
		if (!m_BodyValid) {
			throw Exceptions::Box2DBodyIdInvalid("The m_BodyId Box2D Body Id is invalid");
		}

		if (m_ShapeValid) {
			// Replace old shape with new one
			b2DestroyShape(m_ShapeId, false);
		}
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		const b2Circle circle = {
			b2Vec2_zero,
			radius
		};
		m_ShapeId = b2CreateCircleShape(m_BodyId, &shapeDef, &circle);
		m_ShapeValid = true;
		return *this;
	}

	PhysicsComponent& PhysicsComponent::SetOnCollisionCallback(std::function<void(PhysicsCollision)> callback) {
		m_OnCollisionCallback = callback;
		return *this;
	}


	sf::Vector2f PhysicsComponent::GetPosition() const {
		assert(m_BodyValid && "Body must be valid before calling GetPosition");
		if (!m_BodyValid) {
			return sf::Vector2f{ 0.f, 0.f };
		}

		b2Vec2 pos = b2Body_GetPosition(m_BodyId);
		return sf::Vector2f{ pos.x, pos.y };
	};

	sf::Vector2f PhysicsComponent::GetVelocity() const {
		assert(m_BodyValid && "Body must be valid before calling GetVelocity");
		if (!m_BodyValid) {
			return sf::Vector2f{ 0.f, 0.f };
		}

		b2Vec2 velocity = b2Body_GetLinearVelocity(m_BodyId);
		return sf::Vector2f{ velocity.x, velocity.y };
	}

	// Expensive
	Smasher::Degrees PhysicsComponent::GetRotation() const {
		assert(m_BodyValid && "Body must be valid before calling GetAcceleration");
		if (!m_BodyValid) {
			return Smasher::Radians{ 0.f };
		}

		Smasher::Radians radians{ b2Rot_GetAngle(b2Body_GetRotation(m_BodyId)) };
		return Smasher::ToDegrees(radians);
	}

	float PhysicsComponent::GetMass() const {
		assert(m_BodyValid && "Body must be valid before calling GetAcceleration");
		if (!m_BodyValid) {
			return 0.f;
		}

		return b2Body_GetMass(m_BodyId);
	}


	void PhysicsComponent::OnCollide(PhysicsCollision& collision) {
		if (!m_OnCollisionCallback) {
			return;
		}

		m_OnCollisionCallback(collision);
	}

	PhysicsComponent& PhysicsComponent::SetPosition(sf::Vector2f position) {
		assert(m_BodyValid && "Body must be valid before calling SetPosition");
		if (!m_BodyValid) {
			return *this;
		}
		b2Body_SetTransform(m_BodyId, b2Vec2{ position.x, position.y }, b2Body_GetRotation(m_BodyId));
		return *this;
	};

	PhysicsComponent& PhysicsComponent::SetVelocity(sf::Vector2f velocity) {
		assert(m_BodyValid && "Body must be valid before calling SetVelocity");
		if (!m_BodyValid) {
			return *this;
		}

		b2Body_SetLinearVelocity(m_BodyId, b2Vec2{ velocity.x, velocity.y });

		return *this;
	};

	PhysicsComponent& PhysicsComponent::SetRotation(Smasher::Degrees degrees) {
		assert(m_BodyValid && "Body must be valid before calling SetRotation");
		if (!m_BodyValid) {
			return *this;
		}

		b2Rot rotation = b2MakeRot((float)ToRadians(degrees));
		b2Body_SetTransform(m_BodyId, b2Body_GetPosition(m_BodyId), rotation);
		return *this;
	};

	PhysicsComponent& PhysicsComponent::Move(sf::Vector2f delta) {
		assert(m_BodyValid && "Body must be valid before calling Move");
		if (!m_BodyValid) {
			return *this;
		}

		b2Vec2 position = b2Body_GetPosition(m_BodyId);
		b2Body_SetTransform(m_BodyId, position + b2Vec2{ delta.x, delta.y }, b2Body_GetRotation(m_BodyId));

		return *this;
	};

	PhysicsComponent& PhysicsComponent::ApplyImpulse(sf::Vector2f impulse) {
		assert(m_BodyValid && "Body must be valid before calling Move");
		if (!m_BodyValid) {
			return *this;
		}

		b2Body_ApplyForceToCenter(m_BodyId, b2Vec2{ impulse.x ,impulse.y }, true);
		return *this;
	}

	PhysicsComponent& PhysicsComponent::ApplyForce(sf::Vector2f force) {
		assert(m_BodyValid && "Body must be valid before calling Move");
		if (!m_BodyValid) {
			return *this;
		}

		b2Body_ApplyForceToCenter(m_BodyId, b2Vec2{ force.x ,force.y }, true);
		return *this;
	}

	PhysicsComponent& PhysicsComponent::SetPhysicsType(PhysicsType type) {
		assert(m_BodyValid && "Body must be valid before calling Move");
		if (!m_BodyValid) {
			return *this;
		}

		switch (type) {
			case PhysicsType::DYNAMIC:
				b2Body_SetType(m_BodyId, b2BodyType::b2_dynamicBody);
				break;
			case PhysicsType::STATIC:
				b2Body_SetType(m_BodyId, b2BodyType::b2_staticBody);
				break;
			case PhysicsType::KINEMATIC:
				b2Body_SetType(m_BodyId, b2BodyType::b2_kinematicBody);
				break;

		}
		return *this;
	};

}