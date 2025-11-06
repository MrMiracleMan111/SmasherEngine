#include "Smasher/AABBPhysics.h"
namespace Smasher {
	AABBPhysicsComponentManager::AABBPhysicsComponentManager(Layer& state) :
		BaseComponentManager<AABBPhysicsComponent>(state),
		m_b2QueryContext(
			nullptr, // tree pointer
			false, // relevant collision
			0.f, // Seconds passed
			b2Vec2{0.f, 0.f}, // old position
			b2Vec2{0.f, 0.f}, // position
			b2Vec2{0.f, 0.f}, // scale
			b2Vec2{0.f, 0.f}, // Calcualted velocity after collision
			-1, // Proxy of object to solve position for
			*this
		)
	{
		m_b2Allocator = b2CreateArenaAllocator(2048);
		m_b2BVH = b2DynamicTree_Create();
		m_b2QueryContext.tree = &m_b2BVH;
	}

	AABBPhysicsComponentManager::~AABBPhysicsComponentManager() {
		b2DestroyArenaAllocator(&m_b2Allocator);
		b2DynamicTree_Destroy(&m_b2BVH);
	}


	AABBPhysicsComponent& AABBPhysicsComponentManager::_AddComponent(Entity& rEntity) {
		std::size_t index = m_Components.size();
		int b2TreeProxyId = b2DynamicTree_CreateProxy(&m_b2BVH, AABBPhysicsComponent::DEFAULT_AABB, 0, 0);
		auto itr = m_Components.emplace(m_b2BVH, b2TreeProxyId);
		AABBPhysicsComponent& rComponent = *itr;
		m_b2BVH.nodes[b2TreeProxyId].userData = (uint64_t)(uintptr_t)(&rComponent);
		SetComponentStatus(rComponent, ComponentStatus::VALID);
		SetComponentEntity(rComponent, rEntity);
		SetComponentManager(rComponent, *this);
		SetComponentIterator<AABBPhysicsComponent>(rComponent, new typename plf::colony<AABBPhysicsComponent>::iterator(itr));
		CallOnAddComponent(rComponent); // called when component has been initialiazed and added

		return rComponent;
	}

	void AABBPhysicsComponentManager::PostUpdate(Smasher::Millisecond delta) {
		float timeStep = delta.count() / 1000.f;
		for (auto& component : m_Components) {
			if (component.IsStatic()) {
				continue;
			}

			if (component.GetAcceleration().x != 0.f || component.GetAcceleration().y != 0.f) {
				sf::Vector2f change = (component.GetVelocity() * timeStep) + ((0.5f * timeStep * timeStep) * component.GetAcceleration());
				component.SetVelocity(component.GetVelocity() + (component.GetAcceleration() * timeStep));
				//component.Move(change);
			}
			else if (component.GetVelocity().x != 0.f || component.GetVelocity().y != 0.f) {
				sf::Vector2f change = (component.GetVelocity() * timeStep);
				//component.Move(change);
			}


			if (!component.HasPhysicsChanged()) {
				continue;
			}

			component.ClearPhysicsChangeFlag();

			// Collision resolution (set the new current position)
			m_b2QueryContext.relevantCollision = false;
			m_b2QueryContext.timeStep = timeStep;
			m_b2QueryContext.oldPosition = component.Getb2OldPosition();
			m_b2QueryContext.position = component.Getb2Position();
			m_b2QueryContext.scale = component.Getb2Scale();
			m_b2QueryContext.velocity = component.Getb2Velocity();
			m_b2QueryContext.proxyId = component.Getb2TreeProxyId();
			b2DynamicTree_QueryAll(&m_b2BVH, component.Getb2AABB(), AABBPhysicsComponentManager::_CollisionQueryCallback, &m_b2QueryContext);

			component.Setb2OldPosition(component.Getb2Position());
		}
	}

	bool AABBPhysicsComponentManager::_CollisionQueryCallback(int proxyId, uint64_t userData, void* context) {
		b2CollisionQueryContext& queryContext = *(b2CollisionQueryContext*)context;
		AABBPhysicsComponent& component = *(AABBPhysicsComponent*)(queryContext.tree->nodes[queryContext.proxyId].userData);
		// Self collision
		queryContext.relevantCollision = true;
		if (proxyId == queryContext.proxyId) {
			queryContext.relevantCollision = false;
			return true;
		}
		_ResolveCollision(component, queryContext, proxyId);

		return true;
	}

	bool raycastAABB(b2Vec2& contactPoint, b2Vec2& normal, float& percentDist, b2Vec2 start, b2Vec2 end, b2AABB collider) {
		b2Vec2 dir = b2Normalize(end - start);

		float t_min = -std::numeric_limits<float>::infinity();
		float t_max = std::numeric_limits<float>::infinity();

		// X axis
		{
			float t1 = (collider.lowerBound.x - start.x) / dir.x;
			float t2 = (collider.upperBound.x - start.x) / dir.x;

			float t_near = std::min(t1, t2);
			float t_far = std::max(t1, t2);

			if (t_near > t_min) {
				t_min = t_near;
				normal = (t1 < t2) ? b2Vec2{ -1.f, 0.f } : b2Vec2{ 1.f, 0.f };
			}
			t_max = std::min(t_max, t_far);
		}

		// Y axis
		{
			float t1 = (collider.lowerBound.y - start.y) / dir.y;
			float t2 = (collider.upperBound.y - start.y) / dir.y;

			float t_near = std::min(t1, t2);
			float t_far = std::max(t1, t2);

			if (t_near > t_min) {
				t_min = t_near;
				normal = (t1 < t2) ? b2Vec2{ 0.f, -1.f } : b2Vec2{ 0.f, 1.f };
			}
			t_max = std::min(t_max, t_far);
		}

		// Check if ray actually hits
		if (t_max < t_min || t_max < 0.f) {
			return false;
		}

		// Clamp tmin to 0 only at the end
		float tmp = std::max(0.f, t_min);
		contactPoint = start + (dir * tmp);
		float contactDist = b2Distance(contactPoint, start);
		float length = b2Distance(start, end);

		percentDist = contactDist / length;

		return true;
	}

	void AABBPhysicsComponentManager::_ResolveCollision(AABBPhysicsComponent& component, b2CollisionQueryContext& context, int proxyId) {
		b2TreeNode& node = context.manager.m_b2BVH.nodes[proxyId];

		AABBPhysicsComponent& obstacle = *(AABBPhysicsComponent*)node.userData;
		float percentDist = 2.0f;
		b2Vec2 contactPoint, normal;
		b2AABB fatCollider = node.aabb;
		fatCollider.lowerBound = fatCollider.lowerBound - (context.scale * 0.5f);
		fatCollider.upperBound = fatCollider.upperBound + (context.scale * 0.5f);

		if (!raycastAABB(contactPoint, normal, percentDist, context.oldPosition, context.position, fatCollider)) {
			return;
		}

		b2Vec2 invNormal{ std::abs(normal.y), std::abs(normal.x) };
		b2Vec2 dir = b2Normalize(context.position - context.oldPosition);
		float length = b2Distance(context.oldPosition, context.position);
		invNormal = b2Abs(invNormal);
		context.position = contactPoint + (1.0f - percentDist) * length * b2Mul(invNormal, dir);
		component.MoveTo(sf::Vector2f(context.position.x, context.position.y));
		//component.SetPosition(sf::Vector2f(contactPoint.x, contactPoint.y));
	}

}