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
	class IComponent;

	template<typename T>
	concept IComponentType = std::is_base_of_v<IComponent, T>;

	class SMASHER_API Entity final {

	public:
		Entity(GameState& state, UUID uuid) : m_GameState(state), m_UUID(uuid) {};
		Entity(const Entity&) = default;
		Entity(Entity&&) = default;
		Entity& operator =(const Entity&) = default;
		Entity& operator =(Entity&&) = default;
		~Entity();

		UUID GetUUID() { return m_UUID; }

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
		std::unordered_map<std::type_index, std::reference_wrapper<IComponent>> m_ComponentsByType;
	};
}

#include "Entity.inl"