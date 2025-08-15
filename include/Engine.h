#pragma once
#include <unordered_map>
#include <optional>
#include <mutex>
#include <atomic>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "Base.h"
#include "EngineConfig.h"
#include "ResourceManager.h"
#include "EventManager.h"
#include "Events.h"

namespace Smasher {
	class GameState;

	class SMASHER_API Engine final {
	public:
		Engine();
		Engine(int width, int height);
		Engine(int width, int height, const sf::ContextSettings& settings);
		~Engine();

		void Init();
		void Run();
		void Shutdown();
		void Update(Millisecond delta);
		void Render(sf::RenderWindow& rWindow);

		template<class T, typename... Args>
		T& AddState(Args&&... componentArgs);

		template<class T>
		T& GetState() const;

		template<class T>
		bool HasState() const;

		sf::RenderWindow& GetWindow();
		EventManager& GetEventManager();
		ResourceManager& GetResourceManager();

		void SetUpdateInterval(Millisecond interval) { m_UpdateInterval = interval; }
		void SetRenderInterval(Millisecond interval) { m_RenderInterval = interval; }
		bool IsRunning() const { return m_RunningAtomic; }

	private:
#ifdef BENCHMARK
		void BENCHMARK_LogAccumulatedTime();
#endif

		void OnWindowClose(const Events::WindowCloseEvent& event);

		std::unordered_map<std::type_index, std::unique_ptr<GameState>> m_GameStateByType;
		sf::RenderWindow m_Window;
		std::atomic_bool m_RunningAtomic = true;
		bool m_IsWindowOpen = false; // Needed to prevent double delete on Window Context
		std::mutex m_WindowMutex; // Prevent double delete on Window Context
		EventManager m_EventManager;
		ResourceManager m_ResourceManager;
		Millisecond m_UpdateInterval = EngineConfig::UPDATE_INTERVAL;
		Millisecond m_RenderInterval = EngineConfig::RENDER_INTERVAL;

		// Cached for internal update/render speed checks
		std::chrono::time_point<std::chrono::system_clock> m_UpdateTimestamp;
		std::chrono::time_point<std::chrono::system_clock> m_RenderTimestamp;

		// Handles must be deconstructed first
		EventSubscriptionHandle m_WindowCloseHandle;
	};
}

#include "Engine.inl"