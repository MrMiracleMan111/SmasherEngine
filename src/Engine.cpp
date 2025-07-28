#include <format>
#include <exception>
#include <stdexcept>
#include <iostream>
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

			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			Millisecond updateTimer{ 0 };
			Millisecond renderTimer{ 0 };

			while (m_Window.isOpen() and m_RunningAtomic) {
				std::chrono::time_point<std::chrono::system_clock> tmp = std::chrono::system_clock::now();
				Millisecond diff = std::chrono::duration_cast<std::chrono::milliseconds>(tmp - now);
				now = tmp;
				sf::Event event;
				while (m_Window.pollEvent(event)) {
					if (event.type == sf::Event::Closed) {
						m_Window.close();
					}
				}
				updateTimer += diff;
				renderTimer += diff;

				if (updateTimer >= m_UpdateInterval) {
					Update(Millisecond{ updateTimer });
					updateTimer = Millisecond{ 0 };
				}
				if (renderTimer >= m_RenderInterval) {
					Render(m_Window);
					renderTimer = Millisecond{ 0 };
				}

				Millisecond minInterval = std::min(m_UpdateInterval, m_RenderInterval);
				std::chrono::milliseconds loopTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now);
				Millisecond sleepTime = std::max(minInterval - loopTime, std::chrono::milliseconds::zero());
				std::this_thread::sleep_for(sleepTime);
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
				pGameState->Update(delta);
				pGameState->UpdateComponents(delta);
			}
		}
	}

	void Engine::Render(sf::RenderWindow& window) {
		m_Window.clear();
		for (const auto& [_, pGameState] : m_GameStateByType) {
			if (pGameState->GetStatus() == GameStateStatus::ACTIVE) {
				pGameState->Render(window);
				pGameState->RenderComponents(window);
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