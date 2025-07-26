#pragma once
#include <vector>
#include <memory>
#include "Base.h"
#include "Component.h"

/**
Design Criteria

Component instance solely house data
ComponentManager contains the logic for components

Components should be stored within the manager
Ideally Components are stored adjacent in memory (using something like an array)

*/

namespace Smasher {
	class Entity;
	class GameState;
	class SMASHER_API ComponentManager {
		friend class Entity;
	public:
		ComponentManager(GameState& state) : m_GameState(state) {};
		virtual void Update(Millisecond delta) = 0;
		virtual ~ComponentManager() = default;
		ComponentManager() = delete;
		ComponentManager(ComponentManager&) = delete;
		ComponentManager(Component&&) = delete;
		ComponentManager& operator=(ComponentManager&) = delete;
		ComponentManager& operator=(ComponentManager&&) = delete;

	protected:
		virtual void RemoveComponent(std::unique_ptr<Component>& component) {
			if (!component->IsValid()) {
				throw Exceptions::ComponentInvalid("Component was probably already destroyed");
			}

			// Need to do the empty() check to avoid size_t overflow from 
			// the expression m_Components.size() - 1
			if (!m_Components.empty() && (component->GetIndex() < (m_Components.size() - 1))) {
				// Move last element into this index and pop off last element
				size_t index = component->GetIndex();
				std::swap(component, m_Components.back());
				component->SetIndex(index);
			}

			m_Components.pop_back();
		}

		virtual std::unique_ptr<Component>& AddComponent(std::unique_ptr<Component> component) {
			component->MakeValid();
			m_Components.push_back(std::move(component));
			return m_Components.back();
		}

	private:
		const GameState& m_GameState;
		std::vector<std::unique_ptr<Component>> m_Components;
	};
}