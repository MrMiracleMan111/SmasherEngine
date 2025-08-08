#include "EventManager.h"
#include "Events.h"
namespace Smasher {
	EventManager::~EventManager() {
		Shutdown();
	}

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
				subsription.Callback(*pEvent.get());
			}
		}
		m_EventQueue.clear();
	}

	void EventManager::DispatchAsync() {
		for (const auto& pEvent : m_AsyncEventQueue) {
			std::size_t index = static_cast<std::size_t>(pEvent->GetEventType());
			auto& subscriptionList = m_AsyncEventSubscriptionsByType[index];
			for (auto& subsription : subscriptionList) {
				subsription.Callback(*pEvent.get());
			}
		}
		m_AsyncEventQueue.clear();
	}

	void EventManager::AsyncEventConsumer() {
		std::unique_lock lock(m_AsyncEventsMutex);
		while (m_AsyncRunning) {
			m_AsyncEventsCV.wait(lock, [=] { return (m_AsyncEventSwapQueue.size() > 0 || !m_AsyncRunning); });
			m_AsyncEventSwapQueueMutex.lock();
			std::swap(m_AsyncEventSwapQueue, m_AsyncEventQueue);
			DispatchAsync();
			m_AsyncEventSwapQueueMutex.unlock();
		}
	}

	void EventManager::Shutdown() {
		m_AsyncRunning = false;
		if (m_AsyncEventConsumerThread.joinable()) {
			std::unique_lock lock(m_AsyncEventsMutex);
			m_AsyncEventsCV.notify_all();
			lock.unlock();
			m_AsyncEventConsumerThread.join();
		}
	}
}