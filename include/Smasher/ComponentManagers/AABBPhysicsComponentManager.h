#pragma once
#include "Smasher/Base.h"
#include "Smasher/ComponentManagers/BaseComponentManager.h"

#include "Smasher/Util/Box2DUtil/arena_allocator.h"
#include "Smasher/Util/Box2DUtil/collision.h"

namespace Smasher {
	class AABBPhysicsComponentManager;

	struct b2CollisionQueryContext
	{
		b2DynamicTree* tree;
		bool relevantCollision = false;
		float timeStep = 0.f; // Seconds passed
		b2Vec2 oldPosition;
		b2Vec2 position;
		b2Vec2 scale;
		b2Vec2 velocity; // Calcualted velocity after collision
		int proxyId = -1; // Proxy of object to solve position for
		const AABBPhysicsComponentManager& manager;
	};

	class SMASHER_API AABBPhysicsComponentManager : public BaseComponentManager<AABBPhysicsComponent> {
	public:
		AABBPhysicsComponentManager() = delete;
		AABBPhysicsComponentManager(Layer& state);
		AABBPhysicsComponentManager(const AABBPhysicsComponentManager&) = default;
		~AABBPhysicsComponentManager();

		void PostUpdate(Smasher::Millisecond delta) override;

		template<typename... Args>
		AABBPhysicsComponent& AddComponent(Entity& rEntity, Args&&... args);

	private:
		// Run for each overlap between objects
		static bool _CollisionQueryCallback(int proxyId, uint64_t userData, void* context);
		static void _ResolveCollision(AABBPhysicsComponent& component, b2CollisionQueryContext& context, int proxyId);
		AABBPhysicsComponent& _AddComponent(Entity& rEntity);
		b2ArenaAllocator m_b2Allocator;
		b2DynamicTree m_b2BVH; // Bounded Volume Hierarchy
		b2CollisionQueryContext m_b2QueryContext;
	};
}