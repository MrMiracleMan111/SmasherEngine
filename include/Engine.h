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
	class Layer;
	struct LayerTransition;


	// Non-Copyable
	class SMASHER_API Engine final {
		friend class EventManager;
	public:
		Engine();
		Engine(int width, int height);
		Engine(int width, int height, const sf::ContextSettings& settings);
		~Engine();

		// non-copyable, moveable
		Engine(const Engine&) = delete;
		Engine(Engine&& other) noexcept;
		Engine& operator =(const Engine&) = delete;
		Engine& operator =(Engine&& other) noexcept;

		static Engine CreateHeadless();

		void Init();
		void Run();
		void Shutdown();
		void Update(Millisecond delta);
		void Render(sf::RenderWindow& rWindow);

		// Creates layer and adds it to transition list
		template<class T, typename... Args>
		T& PushLayer(Args&&... componentArgs);

		template<class T>
		T& GetLayer() const;

		std::vector<std::pair<std::type_index, std::unique_ptr<Layer>>>::iterator TopLayerItr();

		template<class T>
		bool HasLayer() const;

		sf::RenderWindow& GetWindow();
		EventManager& GetEventManager();
		ResourceManager& GetResourceManager();

		void SetUpdateInterval(Millisecond interval) { m_UpdateInterval = interval; }
		void SetRenderInterval(Millisecond interval) { m_RenderInterval = interval; }
		bool IsRunning() const { return m_RunningAtomic; }
		bool IsHeadless() const { return m_Headless; }

	protected:
		std::vector<std::pair<std::type_index, std::unique_ptr<Layer>>> m_LayerStack;

	private:
#ifdef BENCHMARK
		void BENCHMARK_LogAccumulatedTime();
#endif
		Engine(bool headless); // Headless constructor
		void OnWindowClose(Events::WindowCloseEvent& event);

		void AddLayer(LayerTransition& transition);
		void RemoveLayer(LayerTransition& transition);

		void HandleLayerTransitions(); // Handles all layer transitions

		bool m_Valid = true; // Becomes false if this object is moved
		std::vector<LayerTransition> m_LayerTransitions;
		std::unique_ptr<sf::RenderWindow> m_Window = nullptr;
		std::atomic_bool m_RunningAtomic = false;
		bool m_Headless = false;
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