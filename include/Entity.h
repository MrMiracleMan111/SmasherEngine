#pragma once
#include <list>
#include <memory.h>
#include <unordered_map>
#include <typeindex>
#include <format>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "Base.h"
#include "UUID.h"
#include "GameState.h"

namespace Smasher {

	class IComponent;
	class Engine;

	class SMASHER_API Entity {

	public:
		Entity() = delete;
		Entity(GameState& state, UUID uuid) : m_GameState(state), m_UUID(uuid), m_Engine(state.GetEngine()) {};
		Entity(const Entity& other) : m_GameState(other.m_GameState), m_UUID(other.m_UUID), m_Engine(other.m_Engine) {};
		Entity(Entity&& other) : m_GameState(other.m_GameState), m_UUID(other.m_UUID), m_Engine(other.m_Engine) {};
		Entity& operator =(const Entity& other) = delete;
		Entity& operator =(Entity&&) = delete;
		virtual ~Entity();

		GameState& GetGameState() { return m_GameState; };
		Engine& GetEngine() { return m_Engine; };
		UUID GetUUID() const { return m_UUID; };

		virtual void Init() {}

		template<class T>
		bool HasComponent() {
			const std::type_index index = std::type_index(typeid(T));
			return m_ComponentsByType.find(index) != m_ComponentsByType.end();
		}

		template<IComponentType T, typename... Args>
		T& AddComponent(Args&&... componentArgs);

		template<class T>
		void RemoveComponent();

		template<class T>
		T& GetComponent() const;

	private:
		const UUID m_UUID;
		GameState& m_GameState;
		Engine& m_Engine;
		std::unordered_map<std::type_index, std::reference_wrapper<IComponent>> m_ComponentsByType;
	};
}

#include "Entity.inl"