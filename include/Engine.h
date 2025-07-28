#pragma once
#include <unordered_map>
#include <optional>
#include <mutex>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <atomic>
#include "Base.h"
#include "EngineConfig.h"
#include "GameState.h"

namespace Smasher {
	class SMASHER_API Engine {
	public:
		Engine();
		Engine(int width, int height);
		~Engine() {};

		void Update(Millisecond delta);
		void Render(sf::RenderWindow& window);

		sf::RenderWindow& GetWindow() { return m_Window; }

		template<class T, typename... Args>
		T& AddState(Args&&... componentArgs) {			
			if (HasState<T>()) {
				std::string exceptionMessage = std::format("Engine already has State of type {}", typeid(T).name());
				throw Exceptions::GameStateDuplicate(exceptionMessage);
			}
			std::type_index index = std::type_index(typeid(T));
			auto pState = std::make_unique<T>(*this, std::forward<Args>(componentArgs)...);
			T& rState = *pState;
			m_GameStateByType.insert({ index, std::move(pState) });
			rState.Init();
			return rState;
		}

		void Run();
		void Shutdown();

		template<class T>
		T& GetState() const {
			auto itr = m_GameStateByType.find(std::type_index(typeid(T)));
			if (itr == m_GameStateByType.end()) {
				std::string exceptionMessage = std::format("Engine has no GameState of type {}", typeid(T).name());
				throw Exceptions::GameStateNotFound(exceptionMessage);
			}
			return static_cast<T&>(*itr->second.get());
		}

		template<class T>
		bool HasState() const {
			return m_GameStateByType.find(std::type_index(typeid(T))) != m_GameStateByType.end();
		}

		bool IsRunning() const { return m_RunningAtomic; }

		void SetUpdateInterval(Millisecond interval) { m_UpdateInterval = interval; }
		void SetRenderInterval(Millisecond interval) { m_RenderInterval = interval; }

	private:
		std::unordered_map<std::type_index, std::unique_ptr<GameState>> m_GameStateByType;
		sf::RenderWindow m_Window;
		std::atomic_bool m_RunningAtomic = true;
		bool m_IsWindowOpen = false; // Needed to prevent double delete on Window Context
		std::mutex m_WindowMutex; // Prevent double delete on Window Context

		Millisecond m_UpdateInterval = EngineConfig::UPDATE_INTERVAL;
		Millisecond m_RenderInterval = EngineConfig::RENDER_INTERVAL;
	};
}