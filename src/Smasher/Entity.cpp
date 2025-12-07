#include <unordered_map>
#include "Smasher/Entity.h"
#include "Smasher/Layer.h"

namespace Smasher {
	Entity::~Entity() {
		for (auto &[index, component] : m_ComponentsByType) {
			IComponentManager &manager = component.get().GetManager();
			manager.RemoveComponent(component);
		}
		m_ComponentsByType.clear();
	}
}