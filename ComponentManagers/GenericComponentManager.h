#pragma once
#include "Base.h"
#include "ComponentManager.h"
#include "Component.h"


/**
	Instead of component logic being handled in the manager, this manager expects
	components to have a static method "StaticUpdateComponent" to update component.

	The effect is offloading the update code to the component to remove the need for
	new Component Managers for every component (especially useful for simple components with little logic)


	This Component Manager will also check if the template T has a "StaticUpdateComponent" before trying
	to run it. This removes need for "StaticUpdateComponent" implementation on purely
	data components (Position, Rotation, etc.) where it wouldn't do anything.
*/

namespace Smasher {
	template <class T>
	concept HasStaticUpdateComponent = requires() {
		T::StaticUpdateComponent;
	};

	template <class T>
	class GenericComponentManager : public ComponentManager {
	public:
		GenericComponentManager(GameState& state) : ComponentManager(state) {};
		GenericComponentManager& operator= (GenericComponentManager&&) = default;

		void Update(Millisecond delta) {
			if constexpr (HasStaticUpdateComponent<T>) {
				for (auto itr : m_Components) {
					T::StaticUpdateComponent(itr, delta);
				}
			}
		}
	};
}