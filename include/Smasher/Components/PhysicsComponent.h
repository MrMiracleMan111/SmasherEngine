#pragma once
#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "box2d/types.h"
#include "Smasher/Base.h"
#include "Smasher/IComponent.h"
#include "Smasher/Layer.h"

namespace Smasher {
	class PhysicsComponentManager;
	class PhysicsComponent;

	struct SMASHER_API PhysicsCollision {
		PhysicsComponent &other; // Object collided with
		sf::Vector2f normal; // surface normal
	};

	enum SMASHER_API PhysicsType {
		STATIC, // Does not move
		KINEMATIC, // Ignores collision, affected by velocity and acceleration
		DYNAMIC // Affected by collision, affected by velocity and acceleration
	};

	class SMASHER_API PhysicsComponent : public IComponent {
		friend class PhysicsComponentManager;
		SMASHER_USE_COMPONENT_MANAGER(PhysicsComponentManager)

	public:
		PhysicsComponent();
		~PhysicsComponent();
		PhysicsComponent(const PhysicsComponent &other) = delete;
		PhysicsComponent(PhysicsComponent &&other) noexcept;
		PhysicsComponent& operator = (const PhysicsComponent &other) = delete;
		PhysicsComponent& operator = (PhysicsComponent &&other) noexcept;

		void OnAddComponent() override;

		PhysicsComponent& SetPhysicsType(PhysicsType type);

		PhysicsComponent& SetPosition(sf::Vector2f position);

		PhysicsComponent& SetVelocity(sf::Vector2f velocity);

		PhysicsComponent& SetRotation(Smasher::Degrees degrees);

		PhysicsComponent& Move(sf::Vector2f delta);

		PhysicsComponent& ApplyImpulse(sf::Vector2f impulse);

		PhysicsComponent& ApplyForce(sf::Vector2f force);

		PhysicsComponent& SetOnCollisionCallback(std::function<void(PhysicsCollision)> callback);

		PhysicsComponent& UseShapeDef(b2ShapeDef &def);

		PhysicsComponent& UseRectCollider(float width, float height);

		PhysicsComponent& UseCircleCollider(float radius);

		template<class T>
		PhysicsComponent& SetOnCollisionCallback(void (T:: *method)(PhysicsCollision), T *instance);

		const b2BodyId& GetBodyId() const { return m_BodyId; }
		PhysicsType GetPhysicsType() { return m_PhysicsType; }
		sf::Vector2f GetPosition() const;
		sf::Vector2f GetVelocity() const;
		Smasher::Degrees GetRotation() const;
		float GetMass() const;

	protected:
		void OnCollide(PhysicsCollision &collision);

	private:
		PhysicsType m_PhysicsType = PhysicsType::STATIC;
		b2BodyId m_BodyId{ 0 };
		b2ShapeId m_ShapeId{ 0 }; // Collider
		b2ShapeDef m_ShapeDef;
		bool m_BodyValid = false;
		bool m_ShapeValid = false;
		std::function<void(PhysicsCollision)> m_OnCollisionCallback;
	};
}

#include "Smasher/Components/PhysicsComponent.inl"