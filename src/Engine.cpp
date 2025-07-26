#include "Engine.h"
#include "Exceptions.h"
#include <format>

namespace Smasher {
	Engine::Engine() : m_Window(sf::VideoMode(640, 480), "Smasher Engine") {

	}

	Engine::Engine(int width, int height) : m_Window(sf::VideoMode(width, height), "Smasher Engine") {
	}

	void Engine::AddState(uint64_t id, GameState& state) {
		throw Exceptions::GameStateDuplicateUUID(std::format("State with id: {} already exists", id));
	}

	GameState& Engine::GetState(uint64_t id) {
		throw Exceptions::GameStateNotFound(std::format("No state with id: {}", id));
	}

	void Engine::Run() {
		while (m_Window.isOpen() and running) {
			sf::Event event;
			while (m_Window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					m_Window.close();
				}
			}
			Render(m_Window);
		}
	}

	void Engine::Update(Millisecond delta) {

	}

	void Engine::Render(sf::RenderWindow& window) {
		m_Window.clear();
		for (const auto& [_, state] : m_GameStateMap) {
			if (state->GetStatus() == GameStateStatus::ACTIVE) {
				state->Render(window);
			}
		}
		m_Window.display();
	}

	void Engine::Shutdown() {
		running = false;
	}
}