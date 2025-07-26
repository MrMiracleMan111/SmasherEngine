#pragma once
#include <unordered_map>
#include <optional>
#include <type_traits>
#include <typeindex>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "UUID.h"
#include "EventManager.h"


namespace Smasher {
	class Engine;
	class Entity;
	class ComponentManager;

	template <class T>
	class GenericComponentManager;
	class IRenderable;


	enum GameStateStatus {
		ACTIVE,
		PAUSED
	};

	// Retrieves and lazily loads managers
	template <class T>
	concept HasStaticInstantiateManager = requires() {
		T::StaticInstantiateManager;
	};

	template <class T>
	concept HasStaticRenderComponent = requires() {
		T::StaticRenderComponent;
	};

	class SMASHER_API GameState {
	public:
		GameState(Engine& engine);
		virtual ~GameState();
		GameState() = delete;
		GameState(const GameState&) = delete;
		GameState& operator=(const GameState&) = delete;
		GameState& operator=(const GameState&&) noexcept = delete;

		virtual void Reset() = 0;
		virtual void Render(sf::RenderWindow& window);

		void Update(Millisecond delta);
		void Activate() { m_Status = GameStateStatus::ACTIVE; };
		void Pause() { m_Status = GameStateStatus::PAUSED; };
		bool GetStatus() { return m_Status; };
		Entity& GetEntity(UUID uuid);
		bool HasEntity(UUID uuid);

		EventManager& GetEventManager();

		template <class T>
		void LoadComponentManager();

		template <class T, typename... Args>
		T& AddEntity(Args&&... componentArgs);

		template <class T>
		ComponentManager& GetComponentManager();

	private:
		std::unordered_map<UUID, std::unique_ptr<Entity>> m_EntityMap;
		std::unordered_map<std::type_index, std::unique_ptr<ComponentManager>> m_ComponentManagers;
		std::vector<IRenderable*> m_ComponentRenderableManagers; // Subset of component managers with rendering capabilities
		GameStateStatus m_Status = GameStateStatus::PAUSED;
		Engine& m_Engine;
		EventManager m_EventManager;
	};
}

#include "GameState.inl"
