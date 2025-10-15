#pragma once

namespace Smasher {
	// Defaults to Synchronous
	template<class T, typename... Args>
	void EventManager::Publish(Args&&... eventArgs) {
		static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");
		std::shared_ptr<T> pEvent = std::make_shared<T>(std::chrono::system_clock::now(), std::forward<Args>(eventArgs)...);
		m_EventQueue.push_back(pEvent);

		std::unique_lock lock(m_AsyncEventSwapQueueMutex);
		m_AsyncEventSwapQueue.push_back(pEvent);
		lock.unlock();

		std::unique_lock notifyLock(m_AsyncEventsMutex);
		m_AsyncEventsCV.notify_all();
	};
}