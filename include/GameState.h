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

	class SMASHER_API GameState {
	public:
		virtual ~GameState();
		GameState() = delete;
		GameState(const GameState&) = delete;
		GameState& operator=(const GameState&) = delete;
		GameState& operator=(const GameState&&) noexcept = delete;

		virtual void Reset() {};
		virtual void Render(sf::RenderWindow& window) {};
		virtual void Update(Millisecond delta) {};

		void RenderComponents(sf::RenderWindow& window);
		void UpdateComponents(Millisecond delta);
		void Activate() { m_Status = GameStateStatus::ACTIVE; };
		void Pause() { m_Status = GameStateStatus::PAUSED; };
		bool GetStatus() { return m_Status; };
		Entity& GetEntity(UUID uuid) const;
		bool HasEntity(UUID uuid) const;

		EventManager& GetEventManager();

		template <class T>
		void LoadComponentManager();

		template <class T, typename... Args>
		T& AddEntity(Args&&... componentArgs);

		template <class T>
		ComponentManager& GetComponentManager();

	protected:
		GameState(Engine& engine);
		Engine& m_Engine;

	private:
		std::unordered_map<UUID, std::unique_ptr<Entity>> m_EntityMap;
		std::unordered_map<std::type_index, std::unique_ptr<ComponentManager>> m_ComponentManagers;
		std::vector<IRenderable*> m_ComponentRenderableManagers; // Subset of component managers with rendering capabilities
		GameStateStatus m_Status = GameStateStatus::PAUSED;
		EventManager m_EventManager;
	};
}

#include "GameState.inl"
