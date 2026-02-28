#include <unordered_map>
#include "Smasher/Entity.h"
#include "Smasher/Layer.h"
#include "Smasher/IComponent.h"
#include "Smasher/IComponentManager.h"

namespace Smasher {
	Entity::Entity(Layer& state, UUID uuid) noexcept : m_LayerRef(state), m_Uuid(uuid), m_Engine(state.GetEngine()) {};

	Entity::Entity(Entity&& other) noexcept : m_LayerRef(other.m_LayerRef), m_Uuid(other.m_Uuid), m_Engine(other.m_Engine) {};

	Entity::~Entity() {
		for (auto &[index, component] : m_ComponentsByType) {
			IComponentManager &manager = component.get().GetManager();
			manager.RemoveComponent(component);
		}
		m_ComponentsByType.clear();
	}
}