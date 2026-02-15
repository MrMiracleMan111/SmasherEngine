#include "Smasher/Base.h"
#include "entt/entt.hpp"

namespace Smasher {
	// System is based off of Blizzard's engine 
	// 
	// Contains the logic for 1 or more Component Types
	// Systems should NOT be stateful
	//
	// Example ISystem::PlayerSystem<transform, player>
	// 
	// Static Factory method
	// PlayerSystem::Register<transform, player>();
	class ISystem {
		virtual void Update(Smasher::Millisecond delta);
		
	private:
		entt::registry m_Registry;
	};
}

#include "Smasher/ISystem.inl"