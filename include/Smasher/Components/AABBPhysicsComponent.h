#pragma once
#include "Smasher/Base.h"
#include "Smasher/IComponent.h"
#include "Smasher/Layer.h"
#include "Smasher/Components/Transform2DWrapper.h"

#include "Smasher/Util/Box2DUtil/base.h"
#include "Smasher/Util/Box2DUtil/math_functions.h"
#include "Smasher/Util/Box2DUtil/collision.h"

namespace Smasher {
	class AABBPhysicsComponentManager;
	class AABBPhysicsComponent;

	struct SMASHER_API AABPPhysicsCollision {
		AABBPhysicsComponent& other; // Object collided with
		sf::Vector2f normal; // surface normal
	};

	enum SMASHER_API AABBPhysicsType {
		STATIC, // Does not move
		KINEMATIC, // Ignores collision, affected by velocity and acceleration
		DYNAMIC // Affected by collision, affected by velocity and acceleration
	};

	class SMASHER_API AABBPhysicsComponent : public IComponent {
		friend class AABBPhysicsComponentManager;
		SMASHER_USE_COMPONENT_MANAGER(AABBPhysicsComponentManager)

	public:
		AABBPhysicsComponent() = delete;
		~AABBPhysicsComponent();
		AABBPhysicsComponent(b2DynamicTree& BVH, int b2TreeProxyId);

		AABBPhysicsComponent& SetPhysicsType(AABBPhysicsType type);

		// Uses center point of rectangle for movement
		// Follows collision rules
		AABBPhysicsComponent& Move(sf::Vector2f delta);

		// Uses center point of rectangle for movement
		// Follows collision rules
		AABBPhysicsComponent& MoveTo(sf::Vector2f position);


		// Uses center point of rectangle for movement
		// Ignores collision
		AABBPhysicsComponent& SetPosition(sf::Vector2f position);

		// Scales from centerpoint of box
		AABBPhysicsComponent& SetScale(sf::Vector2f scale);

		AABBPhysicsComponent& SetVelocity(sf::Vector2f velocity);

		AABBPhysicsComponent& SetAcceleration(sf::Vector2f acceleration);

		AABBPhysicsComponent& SetOnCollisionCallback(std::function<void(AABPPhysicsCollision)> callback);
		template<class T>
		AABBPhysicsComponent& SetOnCollisionCallback(void (T::* method)(AABPPhysicsCollision), T* instance);

		AABBPhysicsType GetPhysicsType() { return m_PhysicsType; }
		sf::Vector2f GetPosition() const;
		sf::Vector2f GetScale() const;
		sf::Vector2f GetOldPosition() const;
		b2Vec2 Getb2Scale() const { return m_Scale; };
		b2Vec2 Getb2Position() const { return m_Position; };
		sf::Vector2f GetVelocity() const;
		sf::Vector2f GetAcceleration() const;
		b2Vec2 Getb2Acceleration() const { return m_Acceleration; };
		b2Vec2 Getb2Velocity() const { return m_Velocity; };

		static inline const b2AABB DEFAULT_AABB = b2AABB{ b2Vec2 {0, 0}, b2Vec2{100, 100} };
	
	protected:
		int Getb2TreeProxyId() const { return m_b2TreeProxyId; }
		b2AABB Getb2AABB() const;
		inline void ClearPhysicsChangeFlag() { m_PhysicsChanged = false; }
		bool HasPhysicsChanged() const { return m_PhysicsChanged; }
		b2Vec2 Getb2OldPosition() const { return m_OldPosition; }
		void Setb2OldPosition(b2Vec2 position) { m_OldPosition = position; }
		void OnCollide(AABPPhysicsCollision& collision);
	private:
		b2DynamicTree& m_b2BVHRef;
		std::function<void(AABPPhysicsCollision&)> m_OnCollisionCallback;
		b2Vec2 m_Scale;
		b2Vec2 m_Position;
		b2Vec2 m_OldPosition; // Used and set by manager
		b2Vec2 m_Velocity;
		b2Vec2 m_Acceleration; // Gravity vector
		int m_b2TreeProxyId = -1;
		bool m_PhysicsChanged = false;
		AABBPhysicsType m_PhysicsType = AABBPhysicsType::STATIC;
	};
}

#include "Smasher/Components/AABBPhysicsComponent.inl"