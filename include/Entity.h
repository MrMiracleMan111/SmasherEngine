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

namespace Smasher {
	class GameState;
	class Component;
	class SMASHER_API Entity {

	public:
		Entity(GameState& state, UUID uuid) : m_GameState(state), m_UUID(uuid) {};
		~Entity();

		UUID GetUUID() { return m_UUID; }

		template<class T>
		bool HasComponent() {
			const std::type_index index = std::type_index(typeid(T));
			return m_ComponentsByType.find(index) != m_ComponentsByType.end();
		}

		template<class T, typename... Args>
		T& AddComponent(Args&&... componentArgs);

		template<class T>
		void RemoveComponent();

		template<class T>
		T& GetComponent() const;

	private:
		const UUID m_UUID;
		GameState& m_GameState;
		std::unordered_map<std::type_index, std::reference_wrapper<std::unique_ptr<Component>>> m_ComponentsByType;
	};
}

#include "Entity.inl"