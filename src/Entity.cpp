#include "Entity.h"
#include "GameState.h"

namespace Smasher {
	Entity::~Entity() {
		for (auto& [index, rComponent] : m_ComponentsByType) {
			IComponentManager& rManager = rComponent.get().GetManager();
			rManager.RemoveComponent(rComponent);
		}
		m_ComponentsByType.clear();
	}
}