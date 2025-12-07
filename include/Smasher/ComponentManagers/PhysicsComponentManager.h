#pragma once
#include "box2d/box2d.h"
#include "Smasher/Base.h"
#include "Smasher/Components/PhysicsComponent.h"
#include "Smasher/ComponentManagers/BaseComponentManager.h"

namespace Smasher {
	class SMASHER_API PhysicsComponentManager : public BaseComponentManager<PhysicsComponent> {
	public:
		PhysicsComponentManager() = delete;
		PhysicsComponentManager(Layer &state);
		PhysicsComponentManager(const PhysicsComponentManager&) = default;
		~PhysicsComponentManager();

		void PostUpdate(Smasher::Millisecond delta) override;
	};
}