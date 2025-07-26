#include <optional>
#include "GameState.h"
#include "Entity.h"

namespace Smasher {
	GameState::GameState(Engine& engine) : m_Engine(engine) {
	}

	GameState::~GameState() {

	}

	EventManager& GameState::GetEventManager() { return m_EventManager; }


	void GameState::Render(sf::RenderWindow& window) {

	}

	void GameState::Update(Millisecond delta) {

	}

	Entity& GameState::GetEntity(UUID uuid) {

		auto itr = m_EntityMap.find(uuid);
		if (itr == m_EntityMap.end()) {
			throw Exceptions::GameStateEntityNotFound(std::format("Could not find entity with UUID: {}", (uint64_t)uuid));
		}
		return *itr->second;
	}

	bool GameState::HasEntity(UUID uuid) {
		return (m_EntityMap.find(uuid) != m_EntityMap.end());
	}
}