#include <cassert>
#include "Smasher/Base.h"
#include "Smasher/EngineConfig.h"
#include "Smasher/PhysicsManager.h"

namespace Smasher {
	PhysicsManager::~PhysicsManager() {
		if (m_Initialized) {
			b2DestroyWorld(m_WorldId);
		}
	};

	PhysicsManager::PhysicsManager(PhysicsManager&& other) noexcept :
		m_WorldId(other.m_WorldId), 
		m_Initialized(other.m_Initialized),
		m_Accumulator(other.m_Accumulator)
	{
		other.m_WorldId = b2WorldId{ 0 }; // Nullify world id
		other.m_Initialized = false;
	}

	PhysicsManager& PhysicsManager::operator=(PhysicsManager&& other) noexcept {
		if (this != &other) {
			if (m_Initialized) {
				b2DestroyWorld(m_WorldId);
			}

			m_WorldId = other.m_WorldId;
			m_Accumulator = other.m_Accumulator;
			other.m_Initialized = other.m_Initialized;
			other.m_WorldId = b2WorldId{ 0 }; // Nullify world id
			other.m_Initialized = false;
		}
		return *this;
	}

	void PhysicsManager::Initialize(const b2WorldDef& worldDef) {
		if (m_Initialized) {
			throw Exceptions::Box2DWorldAlreadyCreated("Box 2D World was already created");
		}

		m_WorldId = b2CreateWorld(&worldDef);
		m_Initialized = true;
	}

	void PhysicsManager::Initialize() {
		b2WorldDef world = b2DefaultWorldDef();
		world.gravity = b2Vec2_zero;
		PhysicsManager::Initialize(world);
	}

	void PhysicsManager::Step(Smasher::Millisecond delta) {
		assert(m_Initialized && "Box2D World must be initialized before physics step");
		if (!m_Initialized) {
			return;
		}
		m_Accumulator += (float)delta.count() / 1000.f;

		while (m_Accumulator >= EngineConfig::BOX2D_TIMESTEP) {
			b2World_Step(m_WorldId, EngineConfig::BOX2D_TIMESTEP, EngineConfig::BOX2D_SUBSTEP_COUNT);
			m_Accumulator -= EngineConfig::BOX2D_TIMESTEP;
		}
	}

}
