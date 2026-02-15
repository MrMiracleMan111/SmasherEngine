#pragma once
#include <unordered_map>
#include <optional>
#include <mutex>
#include <atomic>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include "box2d/box2d.h"
#include "Smasher/Base.h"
#include "Smasher/EngineConfig.h"
#include "Smasher/PhysicsManager.h"
#include "Smasher/ResourceManager.h"
#include "Smasher/EventManager.h"
#include "Smasher/JobManager/JobManager.h"
#include "Smasher/Events.h"
#include "Smasher/LayerTransition.h"


namespace Smasher {
	class Layer;
	struct LayerTransition;


	// Non-Copyable
	class SMASHER_API Engine final {

		using LayerStackItr = std::list<std::pair<std::type_index, std::unique_ptr<Layer>>>::iterator;
		using LayerStackConstItr = std::list<std::pair<std::type_index, std::unique_ptr<Layer>>>::const_iterator;

		friend class EventManager;
	public:
		Engine();
		Engine(int width, int height);
		Engine(int width, int height, const sf::ContextSettings &settings);
		~Engine();

		// non-copyable, moveable
		Engine(const Engine&) = delete;
		Engine(Engine &&other) noexcept;
		Engine& operator =(const Engine&) = delete;
		Engine& operator =(Engine &&other) noexcept;

		static Engine CreateHeadless();

		void Init();
		void Run();
		void Shutdown();
		void Update(Millisecond delta);
		void Render();

		// Creates layer and adds it to transition list
		template<class T, typename... Args>
		T& PushLayer(Args&&... componentArgs);

		// Creates layer and adds it to transition list
		template<class T>
		void PopLayer();

		template<class T>
		T& GetLayer() const;

		LayerStackItr TopLayerItr();

		template<class T>
		bool HasLayer() const;

		sf::RenderWindow& GetWindow();
		EventManager& GetEventManager();
		ResourceManager& GetResourceManager();
		PhysicsManager& GetPhysicsManager();
		JobManager& GetJobManager();

		void SetUpdateInterval(Millisecond interval) { m_UpdateInterval = interval; }
		void SetRenderInterval(Millisecond interval) { m_RenderInterval = interval; }
		bool IsRunning() const { return m_RunningAtomic; }
		bool IsHeadless() const { return m_Headless; }

	protected:
		std::list<std::pair<std::type_index, std::unique_ptr<Layer>>> m_LayerStack;
		std::mutex m_LayerTransitionMutex;

	private:
#ifdef BENCHMARK
		void BENCHMARK_LogAccumulatedTime();
#endif
		Engine(bool headless); // Headless constructor
		void OnWindowClose(Events::WindowCloseEvent &event);
		void OnWindowResize(Events::WindowResizeEvent &event);

		void AddLayer(LayerTransition &transition);
		void RemoveLayer(LayerTransition &transition);

		void HandleLayerTransitions(); // Handles all layer transitions


		LayerStackConstItr GetLayerItr(std::type_index index) const;

		bool m_Valid = true; // Becomes false if this object is moved
		std::vector<LayerTransition> m_LayerTransitions;
		std::unique_ptr<sf::RenderWindow> m_WindowPtr = nullptr;
		std::atomic_bool m_RunningAtomic = false;
		bool m_Headless = false;
		bool m_IsWindowOpen = false; // Needed to prevent double delete on Window Context
		std::mutex m_WindowMutex; // Prevent double delete on Window Context
		PhysicsManager m_PhysicsManager;
		EventManager m_EventManager;
		ResourceManager m_ResourceManager;
		JobManager m_JobManager{ 4u };
		sf::View m_WindowView; // Default window view (used for resizing events)
		Millisecond m_UpdateInterval = EngineConfig::UPDATE_INTERVAL;
		Millisecond m_RenderInterval = EngineConfig::RENDER_INTERVAL;

		// Cached for internal update/render speed checks
		std::chrono::time_point<std::chrono::system_clock> m_UpdateTimestamp;
		std::chrono::time_point<std::chrono::system_clock> m_RenderTimestamp;

		// Handles must be deconstructed first
		EventSubscriptionHandle m_WindowCloseHandle;
		EventSubscriptionHandle m_WindowResizeHandle;
	};

	// Engine is non-copyable, moveable
	static_assert(!std::is_copy_constructible_v<Smasher::Engine>);
	static_assert(!std::is_copy_assignable_v<Smasher::Engine>);
	static_assert(std::is_move_constructible_v<Smasher::Engine>);
	static_assert(std::is_move_assignable_v<Smasher::Engine>);
}

#include "Engine.inl"