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
	class IComponentManager;

	template <class T>
	class GenericComponentManager;
	class IRenderable;


	enum GameStateStatus {
		ACTIVE,
		PAUSED
	};

	class SMASHER_API GameState {
		friend class Engine;
	public:
		virtual ~GameState();
		GameState() = delete;
		GameState(const GameState&) = delete;
		GameState& operator=(const GameState&) = delete;
		GameState& operator=(const GameState&&) noexcept = delete;

		virtual void Reset() {};
		virtual void Render(sf::RenderWindow& window) {};
		virtual void PreUpdate(Millisecond delta) {};
		virtual void Update(Millisecond delta) {};

		void Activate() { m_Status = GameStateStatus::ACTIVE; };
		void Pause() { m_Status = GameStateStatus::PAUSED; };
		bool GetStatus() const { return m_Status; };
		Entity& GetEntity(UUID uuid) const;
		bool HasEntity(UUID uuid) const;

		EventManager& GetEventManager();

		template <class T, typename... Args>
		T& AddEntity(Args&&... componentArgs);

		template <class T>
		IComponentManager& GetComponentManager();

	protected:
		GameState(Engine& engine);
		void RenderComponentManagers(sf::RenderWindow& window);
		void UpdateComponentManagers(Millisecond delta);
		void PreUpdateComponentManagers(Millisecond delta);

		void ShutdownEngine();

	private:
		template <class T>
		void LoadComponentManager();

		std::unordered_map<UUID, std::unique_ptr<Entity>> m_EntityMap;
		std::unordered_map<std::type_index, std::unique_ptr<IComponentManager>> m_ComponentManagers;
		std::vector<IComponentManager*> m_ComponentManagersWithRender; // Subset of component managers with Render(sf::Window&) capability
		std::vector<IComponentManager*> m_ComponentManagersWithUpdate; // Subset of component managers with Update(Millisecond) capability
		GameStateStatus m_Status = GameStateStatus::PAUSED;
		EventManager m_EventManager;
		Engine& m_Engine;
	};
}

#include "GameState.inl"
