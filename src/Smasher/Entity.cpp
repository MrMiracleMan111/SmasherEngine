#include <unordered_map>
#include "Smasher/Entity.h"
#include "Smasher/Layer.h"

namespace Smasher {
	Entity::~Entity() {
		for (auto& [index, rComponent] : m_ComponentsByType) {
			IComponentManager& rManager = rComponent.get().GetManager();
			rManager.RemoveComponent(rComponent);
		}
		m_ComponentsByType.clear();
	}
}