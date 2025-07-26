#include <optional>
#include "GameState.h"
#include "Entity.h"

namespace Smasher {
	GameState::GameState(Engine& engine) : m_Engine(engine) {
	}

	GameState::~GameState() {

	}

	EventManager& GameState::GetEventManager() { return m_EventManager; }

	void GameState::RenderComponents(sf::RenderWindow& window) {
		for (auto pRenderable : m_ComponentRenderableManagers) {
			pRenderable->Render(window);
		}
	}

	void GameState::UpdateComponents(Millisecond delta) {
		for (auto& [type, pManager] : m_ComponentManagers) {
			pManager->Update(delta);
			pManager->RemoveMarkedComponents();
		}
	}

	Entity& GameState::GetEntity(UUID uuid) const {

		auto itr = m_EntityMap.find(uuid);
		if (itr == m_EntityMap.end()) {
			throw Exceptions::GameStateEntityNotFound(std::format("Could not find entity with UUID: {}", (uint64_t)uuid));
		}
		return *itr->second;
	}

	bool GameState::HasEntity(UUID uuid) const {
		return (m_EntityMap.find(uuid) != m_EntityMap.end());
	}
}