#pragma once
#include <unordered_map>
#include <optional>
#include <type_traits>
#include <typeindex>
#include <SFML/Window.hpp>
#include "Base.h"
#include "UUID.h"
#include "Engine.h"


namespace Smasher {
	class IComponentManager;
	class Entity;
	class EventManager;
	class ResourceManager;

	template <class T>
	class GenericComponentManager;


	enum LayerStatus {
		ACTIVE,
		PAUSED
	};

	class SMASHER_API Layer {
		friend class Engine;
		friend class EventManager;
	public:
		Layer(Engine& engine) :
			m_Engine(engine),
			m_EventManager(engine.GetEventManager()),
			m_ResourceManager(engine.GetResourceManager()),
			m_UpdateTime(Millisecond::zero()),
			m_RenderTime(Millisecond::zero()) {
		}
		virtual ~Layer();
		Layer() = delete;
		Layer(const Layer&) = delete;
		Layer& operator=(const Layer&) = delete;
		Layer& operator=(const Layer&&) noexcept = delete;

		Millisecond GetUpdateTime() const { return m_UpdateTime; }
		Millisecond GetRenderTime() const { return m_RenderTime; }

		void SetUpdateTime(Millisecond time) { m_UpdateTime = time; }
		void SetRenderTime(Millisecond time) { m_RenderTime = time; }

		// Called once after game state is added to game engine
		virtual void Init() {};

		// User calls this to reset the game state
		virtual void Reset() {};
		virtual void Render(sf::RenderWindow& window) {};
		virtual void PreUpdate(Millisecond delta) {};
		virtual void Update(Millisecond delta) {};

		void Activate() { m_Status = LayerStatus::ACTIVE; };
		void Pause() { m_Status = LayerStatus::PAUSED; };
		bool GetStatus() const { return m_Status; };
		Entity& GetEntity(UUID uuid);
		bool HasEntity(UUID uuid) const;

		EventManager& GetEventManager() { return m_EventManager; }
		Engine& GetEngine() { return m_Engine; }

		template <class T, typename... Args>
		T& AddEntity(Args&&... componentArgs);

		Entity& AddEntity();

		void RemoveEntity(UUID uuid);

        // Component Type <T>
		template <class ComponentType>
		IComponentManager& GetComponentManager();

		std::size_t EntityCount() { return m_EntityMap.size(); }

		// Subscribe to synchronous event handling (immediately after its called)
		template<class T>
		EventSubscriptionHandle Subscribe(std::function<void(T&)> callback);

		// Overload for class memebr function ex:
		// Subscribe<EventType>(&Class::MemberFunc, classInstancePointer);
		template<class T, class C>
		EventSubscriptionHandle Subscribe(void (C::* method)(T&), C* instance);

		// Subscribe to asynchronous event handling (uses separate Event thread)
		template<class T>
		EventSubscriptionHandle SubscribeAsync(std::function<void(T&)> callback);

		template<class T, class C>
		EventSubscriptionHandle SubscribeAsync(void (C::* method)(T&), C* instance);

		void RenderComponentManagers(sf::RenderWindow& window);
		void UpdateComponentManagers(Millisecond delta);
		void PreUpdateComponentManagers(Millisecond delta);

		void ShutdownEngine();

		std::unordered_map<std::type_index, std::list<EventSubscription>>& GetEventSubscriptions() { return m_EventSubscriptionsByType; };
		std::unordered_map<std::type_index, std::list<EventSubscription>>& GetAsyncEventSubscriptions() { return m_AsyncEventSubscriptionsByType; };
	private:
		template <class T>
		void LoadComponentManager();

		std::unordered_map<UUID, std::unique_ptr<Entity>> m_EntityMap;
		std::unordered_map<std::type_index, std::unique_ptr<IComponentManager>> m_ComponentManagers;
		std::vector<IComponentManager*> m_ComponentManagersWithRender; // Subset of component managers with Render(sf::Window&) capability
		std::vector<IComponentManager*> m_ComponentManagersWithUpdate; // Subset of component managers with Update(Millisecond) capability
		LayerStatus m_Status = LayerStatus::PAUSED;
		EventManager& m_EventManager;
		ResourceManager& m_ResourceManager;
		Engine& m_Engine;

		Millisecond m_UpdateTime;
		Millisecond m_RenderTime;
		std::unordered_map<std::type_index, std::list<EventSubscription>> m_EventSubscriptionsByType;
		std::unordered_map<std::type_index, std::list<EventSubscription>> m_AsyncEventSubscriptionsByType;
	};
}

#include "Layer.inl"
