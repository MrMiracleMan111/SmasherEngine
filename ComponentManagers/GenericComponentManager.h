#pragma once
#include <chrono>
#include <iostream>
#include "Smasher/Base.h"
#include ""Smasher/IComponentManager.h""
#include ""Smasher/IComponent.h""
#include "plf_colony.h"

namespace Smasher {

	//////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief Component Manager that automatically selects functionality to match the Component.
	/// Like all @ref IComponentManager instances, this class is Non-Copyable and may only be move-assigned
	/// 
	/// @param T Type of Component housed in the @ref GenericComponentManager
	/// 
	/// The @ref GenericComponentManager is meant to provide an alternative to creating
	/// custom component managers for each new component. The @ref GenericComponentManager
	/// will instead look for the `StaticUpdateComponent` and `StaticRenderComponent` methods
	/// within the custom component class and call them if they exist. Now, instead of creating
	/// additional Component Manager classes, and simple updates and rendering can be defined
	/// withing the component class as the static methods `StaticUpdateComponent` and `StaticRenderComponent`.
	/// Like the @ref BaseComponentManager this class uses @ref plf::colony to house components.
	//////////////////////////////////////////////////////////////////////////////////////////////////
	template <class T>
	class GenericComponentManager final : public IComponentManager {
		static_assert(std::is_base_of_v<IComponent, T>, "T should be derived from IComponent");

		friend class Entity;
	public:
		GenericComponentManager(Layer& state);
		GenericComponentManager() = delete;
		GenericComponentManager(const GenericComponentManager&) = delete;
		GenericComponentManager(GenericComponentManager&&) = delete;
		GenericComponentManager& operator=(const GenericComponentManager&) = delete;
		GenericComponentManager& operator= (GenericComponentManager&&) = default;
		virtual ~GenericComponentManager() = default;

		void PreUpdate(Millisecond delta) override;

		/// @brief Updates all components.
		/// @param delta Time in @ref Millisecond passed since the last update.
		/// 
		/// @ref GenericComponentManager::Update is called only if Component type `T` defines `StaticUpdateComponent(T& self)`
		/// Update order matching component insertion order is not guaranteed.
		void Update(Millisecond delta) override;

		/// @brief Renders all components.
		/// @param rWindow Reference to the `sf::RenderWindow` to draw on.
		/// 
		/// @ref GenericComponentManager::Render is called only if Component type `T` defines `StaticUpdateComponent(T& self)`.
		/// Render order matching component insertion order is not guaranteed.
		void Render(sf::RenderWindow& rWindow);

		/// @brief Renders all components
		/// @param rEntity Entity to attach the component to.
		/// @param args Arguments to pass to the component constructor.
		/// 
		/// @ref GenericComponentManager::Render is called only if Component type `T` defines `StaticUpdateComponent(T& self)`.
		template<typename... Args>
		T& AddComponent(Entity& rEntity, Args&&... args);

		/// @brief Marks components for removal
		/// @param rComponentInterface Reference to @ref IComponent instance to remove.
		/// 
		/// If the component status is @ref ComponentStatus::VALID it will be ignored
		/// and nothing will happen. This method only marks components for removal.
		/// Component removal is handled after @ref GenericComponentManager::Update by the 
		/// `GenericComponentManager::RemoveMarkedComponents` private method.
		void RemoveComponent(IComponent& rComponentInterface);

	private:
		void RemoveMarkedComponents();

		plf::colony<T> m_Components;
		std::vector<typename plf::colony<T>::iterator*> m_ComponentsToRemove; // Components to be removed
	};
}

#include "ComponentManagers/GenericComponentManager.inl"