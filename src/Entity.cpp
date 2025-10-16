#include <unordered_map>
#include "Entity.h"
#include "Layer.h"

namespace Smasher {
	Entity& Entity::operator=(Entity&& other) noexcept
	{
		if (&other != this) {
			m_LayerRef = std::move(other.m_LayerRef);
			m_UUID = other.m_UUID;
			m_Engine = std::move(other.m_Engine)
		}
		return* this;
	}

	Entity::~Entity() {
		for (auto& [index, rComponent] : m_ComponentsByType) {
			IComponentManager& rManager = rComponent.get().GetManager();
			rManager.RemoveComponent(rComponent);
		}
		m_ComponentsByType.clear();
	}
}