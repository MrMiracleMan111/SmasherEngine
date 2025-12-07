#include "Smasher/Physics.h"
namespace Smasher {
	PhysicsComponentManager::PhysicsComponentManager(Layer &state) :
		BaseComponentManager<PhysicsComponent>(state)
	{
	}

	PhysicsComponentManager::~PhysicsComponentManager() {
		m_Components.clear();
	}

	void PhysicsComponentManager::PostUpdate(Smasher::Millisecond delta) {
	}
}