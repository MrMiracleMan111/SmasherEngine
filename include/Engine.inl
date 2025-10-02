#include <memory>
#include "Engine.h"
#include "Layer.h"

namespace Smasher {
	template<class T, typename... Args>
	T& Engine::AddState(Args&&... componentArgs) {
		if (HasState<T>()) {
			std::string exceptionMessage = std::format("Engine already has State of type {}", typeid(T).name());
			throw Exceptions::GameStateDuplicate(exceptionMessage);
		}
		std::type_index index = std::type_index(typeid(T));
		auto pState = std::make_unique<T>(*this, std::forward<Args>(componentArgs)...);
		T& rState = *pState;
		m_GameStateByType.insert({ index, std::move(pState) });
		rState.Init();
		return rState;
	}

	template<class T>
	T& Engine::GetState() const {
		auto itr = m_GameStateByType.find(std::type_index(typeid(T)));
		if (itr == m_GameStateByType.end()) {
			std::string exceptionMessage = std::format("Engine has no GameState of type {}", typeid(T).name());
			throw Exceptions::GameStateNotFound(exceptionMessage);
		}
		return static_cast<T&>(*itr->second.get());
	}

	template<class T>
	bool Engine::HasState() const {
		return m_GameStateByType.find(std::type_index(typeid(T))) != m_GameStateByType.end();
	}
}