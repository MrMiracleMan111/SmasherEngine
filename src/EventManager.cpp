#include "EventManager.h"
#include "Events.h"
namespace Smasher {
	EventManager::EventManager() {}
	EventManager::~EventManager() {}

	void EventManager::Unsubscribe(EventSubscriptionHandle handle) {
		if (!handle.IsValid()) {
			throw Exceptions::EventHandleInvalid("Handle is invalid");
		}
		handle.m_List.erase(handle.m_Itr);
	}

	void EventManager::Dispatch() {
		for (const auto& pEvent : m_EventQueue ) {
			std::size_t index = static_cast<std::size_t>(pEvent->GetEventType());
			auto& subscriptionList = m_EventSubscriptionsByType[index];
			for (auto& subsription : subscriptionList) {
				subsription.Callback(pEvent.get());
			}
		}
		m_EventQueue.clear();
	}
}