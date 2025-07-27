#include <format>
#include <exception>
#include <stdexcept>
#include "Engine.h"
#include "Exceptions.h"
#include "EngineConfig.h"

namespace Smasher {
	Engine::Engine() : m_Window(sf::VideoMode(EngineConfig::WINDOW_WIDTH, EngineConfig::WINDOW_HEIGHT), EngineConfig::TITLE) {

	}

	Engine::Engine(int width, int height) : m_Window(sf::VideoMode(width, height), EngineConfig::TITLE) {
	}

	void Engine::Run() {
		#ifdef NDEBUG
		try {
		#endif
			while (m_Window.isOpen() and m_RunningAtomic) {
				sf::Event event;
				while (m_Window.pollEvent(event)) {
					if (event.type == sf::Event::Closed) {
						m_Window.close();
					}
				}
				Update(Millisecond{ 10 });
				Render(m_Window);
			}
			Shutdown();
		#ifdef NDEBUG
		}
		catch (const std::exception& e) {
			std::cerr << "Exception Thrown: " << e.what() << std::endl;
		}
		#endif
	}

	void Engine::Update(Millisecond delta) {
		for (auto& [_, pGameState] : m_GameStateByType) {
			if (pGameState->GetStatus() == GameStateStatus::ACTIVE) {
				pGameState->PreUpdate(delta);
				pGameState->PreUpdateComponentManagers(delta);
				pGameState->Update(delta);
				pGameState->UpdateComponentManagers(delta);
			}
		}
	}

	void Engine::Render(sf::RenderWindow& window) {
		m_Window.clear();
		for (const auto& [_, pGameState] : m_GameStateByType) {
			if (pGameState->GetStatus() == GameStateStatus::ACTIVE) {
				pGameState->Render(window);
				pGameState->RenderComponentManagers(window);
			}
		}
		m_Window.display();
	}

	void Engine::Shutdown() {
		m_RunningAtomic = false;

		std::scoped_lock lock(m_WindowMutex);
		if (m_IsWindowOpen) {
			m_Window.close();
			m_IsWindowOpen = false;
		}
	}
}