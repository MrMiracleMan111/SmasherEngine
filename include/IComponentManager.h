#pragma once
#include <memory>
#include "plf_colony.h"
#include <SFML/Window.hpp>
#include "IComponent.h"
#include "Base.h"

namespace Smasher {
	class GameState;

	class IComponentManager {
	public:
		IComponentManager() = delete;
		IComponentManager(GameState& state) : m_GameState(state) {}
		virtual void PreUpdate(Millisecond delta) {};
		virtual void Update(Millisecond delta) {}; // Override this to enable component updating
		virtual void Render(sf::RenderWindow& rWindow) {}; // Override this to enable component rendering
		virtual void RemoveComponent(IComponent& component) = 0;

		GameState& GetGameState() { return m_GameState; }

		GameState& m_GameState;
	protected:
		void SetComponentStatus(IComponent& rComponent, ComponentStatus status) { rComponent.SetStatus(status); }
		void SetComponentEntity(IComponent& rComponent, Entity& rEntity) { rComponent.SetEntity(rEntity); }
		void SetComponentManager(IComponent& rComponent, IComponentManager& rManager) { rComponent.SetManager(rManager); }
		void CallOnAddComponent(IComponent& rComponent) { rComponent.OnAddComponent(); } // called when component has been initialiazed and added

		template<class T>
		typename plf::colony<T>::iterator* GetComponentIterator(T& rComponent) {
			return rComponent.GetIterator<T>();
		}

		template<class T>
		void SetComponentIterator(T& rComponent, typename plf::colony<T>::iterator* itrPtr) {
			rComponent.SetIterator<T>(itrPtr);
		}
	};
}