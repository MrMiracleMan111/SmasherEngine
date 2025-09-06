#pragma once
#include "Base.h"
#include "IComponentManager.h"

namespace Smasher {
	class Entity;
	class GameState;

	/*
	* 
	*  This class is meant to be inherited
	* 
	* 
	*/
	template <class T>
	class BaseComponentManager : public IComponentManager {
	static_assert(std::is_base_of_v<IComponent, T>, "T should be derived from IComponent");

		friend class Entity;
	public:
		BaseComponentManager(GameState& state);
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

		void RemoveComponent(IComponent& rComponentInterface);

	protected:
		plf::colony<T> m_Components;

	private:
		void RemoveMarkedComponents();

		void PreUpdate(Millisecond delta);

		void Update(Millisecond delta);

		void Render(sf::RenderWindow& rWindow);

		std::vector<typename plf::colony<T>::iterator*> m_ComponentsToRemove; // Components to be removed
	};
}

#include "ComponentManagers/BaseComponentManager.inl"