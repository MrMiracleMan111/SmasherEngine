#pragma once
#include "Base.h"
#include "IComponentManager.h"

namespace Smasher {
	class Entity;
	class Layer;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	/// @brief Component Manager meant to be used as a base for Custom Component Managers. Unlike the
	/// @ref GenericComponentManager, this component manager does not detect component functionality.
	/// Instead, the @ref BaseComponentManager::Update(Millisecond delta) and @ref BaseComponentManager::Render(sf::RenderWindow& rWindow) methods
	/// should be overrided as needed.
	/// Like all @ref IComponentManager instances, this class is Non-Copyable and may only be move-assigned
	/// 
	/// @param T Type of Component housed in the @ref GenericComponentManager
	/// 
	/// The @ref BaseComponentManager is meant to serve as a base for Custom Component Managers. It
	/// already implements the @link BaseComponentManager::AddComponent AddComponent @endlink and @link BaseComponentManager::RemoveComponent(IComponent&) RemoveComponent @endlink. Unlike the
	/// @ref GenericComponentManager, the @ref BaseComponentManager does not detect component
	/// functionality(`StaticUpdateComponent` and `StaticRenderComponent`). Instead, the 
	/// @ref Update(Millisecond delta) and @ref Render(sf::RenderWindow& rWindow) methods should
	/// be overrided as needed.
	/// 
	/// The @ref BaseComponentManager uses @ref plf::colony to house components.
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	template <class T>
	class BaseComponentManager : public IComponentManager {
	static_assert(std::is_base_of_v<IComponent, T>, "T should be derived from IComponent");

		friend class Entity;
	public:
		BaseComponentManager(Layer& state);
		BaseComponentManager() = delete;
		BaseComponentManager(const BaseComponentManager&) = delete;
		BaseComponentManager(BaseComponentManager&&) = delete;
		BaseComponentManager& operator=(const BaseComponentManager&) = delete;
		BaseComponentManager& operator=(BaseComponentManager&&) = delete;
		virtual ~BaseComponentManager() = default;

		virtual void PreUpdateComponents() {};
		virtual void UpdateComponents(Millisecond delta) {};
		virtual void RenderComponents(sf::RenderWindow& rWindow) {};

		template<typename... Args>
		T& AddComponent(Entity& rEntity, Args&&... args);

		virtual void RemoveComponent(IComponent& rComponentInterface);

		void PreUpdate(Millisecond delta);
		void Update(Millisecond delta);
		void Render(sf::RenderWindow& rWindow);

	protected:
		plf::colony<T> m_Components;

	private:
		void RemoveMarkedComponents();

		std::vector<typename plf::colony<T>::iterator*> m_ComponentsToRemove; // Components to be removed
	};
}

#include "ComponentManagers/BaseComponentManager.inl"