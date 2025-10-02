#include <optional>
#include <unordered_map>
#include "Layer.h"
#include "Entity.h"
#include "Engine.h"

namespace Smasher {
	Layer::~Layer() {
		m_EntityMap.clear();
		m_ComponentManagersWithRender.clear();
		m_ComponentManagersWithUpdate.clear();
		m_ComponentManagers.clear();
	}

	void Layer::RenderComponentManagers(sf::RenderWindow& window) {
		for (IComponentManager* pManager : m_ComponentManagersWithRender) {
			pManager->Render(window);
		}
	}

	void Layer::PreUpdateComponentManagers(Millisecond delta) {
		for (auto& [key, pManager] : m_ComponentManagers) {
			pManager->PreUpdate(delta);
		}
	}

	void Layer::UpdateComponentManagers(Millisecond delta) {
		for (auto pManager : m_ComponentManagersWithUpdate) {
			pManager->Update(delta);
		}
	}

	void Layer::ShutdownEngine() {
		m_Engine.Shutdown();
	}

	Entity& Layer::GetEntity(UUID uuid) {
		auto itr = m_EntityMap.find(uuid);
		if (itr == m_EntityMap.end()) {
			throw Exceptions::LayerEntityNotFound(std::format("Could not find entity with UUID: {}", (uint64_t)uuid));
		}
		return *itr->second;
	}

	bool Layer::HasEntity(UUID uuid) const {
		return (m_EntityMap.find(uuid) != m_EntityMap.end());
	}

	void Layer::RemoveEntity(UUID uuid) {
		if (!HasEntity(uuid)) {
			throw Exceptions::LayerEntityNotFound(std::format("Could not find entity with UUID: {}", (uint64_t)uuid));
		}
		m_EntityMap.erase(uuid);
	}
}