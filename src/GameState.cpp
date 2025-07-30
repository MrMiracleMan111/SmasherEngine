#include <optional>
#include "GameState.h"
#include "Entity.h"
#include "Engine.h"

namespace Smasher {
	GameState::~GameState() {
		m_EntityMap.clear();
		m_ComponentManagersWithRender.clear();
		m_ComponentManagersWithUpdate.clear();
		m_ComponentManagers.clear();
	}

	void GameState::RenderComponentManagers(sf::RenderWindow& window) {
		for (IComponentManager* pManager : m_ComponentManagersWithRender) {
			pManager->Render(window);
		}
	}

	void GameState::PreUpdateComponentManagers(Millisecond delta) {
		for (auto& [key, pManager] : m_ComponentManagers) {
			pManager->PreUpdate(delta);
		}
	}

	void GameState::UpdateComponentManagers(Millisecond delta) {
		for (auto pManager : m_ComponentManagersWithUpdate) {
			pManager->Update(delta);
		}
	}

	void GameState::ShutdownEngine() {
		m_Engine.Shutdown();
	}

	Entity& GameState::GetEntity(UUID uuid) {
		auto itr = m_EntityMap.find(uuid);
		if (itr == m_EntityMap.end()) {
			throw Exceptions::GameStateEntityNotFound(std::format("Could not find entity with UUID: {}", (uint64_t)uuid));
		}
		return *itr->second;
	}

	bool GameState::HasEntity(UUID uuid) const {
		return (m_EntityMap.find(uuid) != m_EntityMap.end());
	}

	void GameState::RemoveEntity(UUID uuid) {
		if (!HasEntity(uuid)) {
			throw Exceptions::GameStateEntityNotFound(std::format("Could not find entity with UUID: {}", (uint64_t)uuid));
		}
		m_EntityMap.erase(uuid);
	}
}