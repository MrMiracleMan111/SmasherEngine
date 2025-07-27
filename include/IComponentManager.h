#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "IComponent.h"
#include "Base.h"

namespace Smasher {
	class GameState;
	template <typename T>
	concept HasStaticInstantiateManager = requires(Smasher::GameState & arg) {
		T::StaticInstantiateManager(arg);
	};

	template <typename T>
	concept HasStaticRenderComponent = requires(T& comp, sf::RenderWindow& arg) {
		{ T::StaticRenderComponent(comp, arg) } -> std::same_as<void>;
	};

	template <typename T>
	concept HasStaticUpdateComponent = requires(T& comp, Smasher::Millisecond arg) {
		{ T::StaticUpdateComponent(comp, arg) } -> std::same_as<void>;
	};

	enum class ComponentManagerCapability {

	};


	template <typename T>
	concept HasRenderCapability = requires(sf::RenderWindow & arg) {
		T::Render(arg);
	};

	template <typename T>
	concept HasUpdateCapability = requires(Millisecond & arg) {
		T::Update(arg);
	};

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