#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
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
		virtual IComponent& AddComponent(IComponent&& component) = 0;

		const GameState& m_GameState;
	protected:
		void SetComponentStatus(IComponent& rComponent, ComponentStatus status) { rComponent.SetStatus(status); }
		void SetComponentIndex(IComponent& rComponent, size_t index) { rComponent.SetIndex(index); }
	};
}