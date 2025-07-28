#include "Entity.h"
#include "GameState.h"

namespace Smasher {
	Entity::~Entity() {
		for (auto& [index, rComponent] : m_ComponentsByType) {
			rComponent.get().GetManager().RemoveComponent(rComponent);
		}
		m_ComponentsByType.clear();
	}
}