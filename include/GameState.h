#pragma once
#include <unordered_map>
#include <optional>
#include <type_traits>
#include <typeindex>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "UUID.h"
#include "EventManager.h"
#include "ResourceManager.h"
#include "Engine.h"


namespace Smasher {
	class IComponentManager;
	class Entity;

	template <class T>
	class GenericComponentManager;


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

		virtual void Init() {};
		virtual void Reset() {};
		virtual void Render(sf::RenderWindow& window) {};
		virtual void PreUpdate(Millisecond delta) {};
		virtual void Update(Millisecond delta) {};

		void Activate() { m_Status = GameStateStatus::ACTIVE; };
		void Pause() { m_Status = GameStateStatus::PAUSED; };
		bool GetStatus() const { return m_Status; };
		Entity& GetEntity(UUID uuid);
		bool HasEntity(UUID uuid) const;

		EventManager& GetEventManager() { return m_EventManager; }
		Engine& GetEngine() { return m_Engine; }

		template <class T, typename... Args>
		T& AddEntity(Args&&... componentArgs);

		void RemoveEntity(UUID uuid);

        // Component Type <T>
		template <class T>
		IComponentManager& GetComponentManager();

	protected:
		GameState(Engine& engine) :
			m_Engine(engine),
			m_EventManager(engine.GetEventManager()),
			m_ResourceManager(engine.GetResourceManager()) {}

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
		EventManager& m_EventManager;
		ResourceManager& m_ResourceManager;
		Engine& m_Engine;
	};
}

#include "GameState.inl"
