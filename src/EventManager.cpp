#include "EventManager.h"
#include "Events.h"
namespace Smasher {
	EventManager::~EventManager() {
		StopAsync();
	}

	EventManager::EventManager(EventManager&& other) noexcept :
		m_EventSubscriptionsByType(std::move(other.m_EventSubscriptionsByType)),
		m_AsyncEventSubscriptionsByType(std::move(other.m_AsyncEventSubscriptionsByType)),
		m_EventQueuePtr(std::move(other.m_EventQueuePtr)),
		m_AsyncEventQueuePtr(std::move(other.m_AsyncEventQueuePtr)),
		m_AsyncEventSwapQueue(std::move(other.m_AsyncEventSwapQueue)),
		m_AsyncRunning(false)
	{
		other.StopAsync();
		m_AsyncRunning = true;
		m_AsyncEventConsumerThread = std::thread(&EventManager::AsyncEventConsumer, this);
	}

	EventManager& EventManager::operator=(EventManager&& other) noexcept
	{
		if (this != &other) {
			// Stop Async events on this
			StopAsync();

			// Wait for async events to finish processing
			other.StopAsync();

			m_EventSubscriptionsByType = std::move(other.m_EventSubscriptionsByType);
			m_AsyncEventSubscriptionsByType = std::move(other.m_AsyncEventSubscriptionsByType);
			m_EventQueuePtr = std::move(other.m_EventQueuePtr);
			m_AsyncEventQueuePtr = std::move(other.m_AsyncEventQueuePtr);
			m_AsyncEventSwapQueue = std::move(other.m_AsyncEventSwapQueue);
			m_AsyncRunning = true;
			m_AsyncEventConsumerThread = std::thread(&EventManager::AsyncEventConsumer, this);
		}
		return *this;
	}

	void EventManager::Unsubscribe(EventSubscriptionHandle& handle) {
		if (!handle.IsValid()) {
			throw Exceptions::EventHandleInvalid("Handle is invalid");
		}
		handle.Invalidate();
		handle.m_SubscriptionListPtr->erase(handle.m_Itr);
	}

	void EventManager::Dispatch() {
		for (const auto& pEvent : *m_EventQueuePtr ) {
			std::size_t index = static_cast<std::size_t>(pEvent->GetEventType());
			auto& subscriptionList = m_EventSubscriptionsByType[index];
			for (auto& subsription : subscriptionList) {
				subsription.Callback(*pEvent.get());
			}
		}
		m_EventQueuePtr->clear();
	}

	void EventManager::DispatchAsync() {
		for (const auto& pEvent : *m_AsyncEventQueuePtr) {
			std::size_t index = static_cast<std::size_t>(pEvent->GetEventType());
			auto& subscriptionList = m_AsyncEventSubscriptionsByType[index];
			for (auto& subsription : subscriptionList) {
				subsription.Callback(*pEvent.get());
			}
		}
		m_AsyncEventQueuePtr->clear();
	}

	void EventManager::AsyncEventConsumer() {
		std::unique_lock lock(m_AsyncEventsMutex);
		while (m_AsyncRunning) {
			m_AsyncEventsCV.wait(lock, [=] { return (m_AsyncEventSwapQueue.size() > 0 || !m_AsyncRunning); });
			m_AsyncEventSwapQueueMutex.lock();
			std::swap(m_AsyncEventSwapQueue, *m_AsyncEventQueuePtr);
			DispatchAsync();
			m_AsyncEventSwapQueueMutex.unlock();
		}
	}

	void EventManager::StopAsync()
	{
		m_AsyncRunning = false;
		if (m_AsyncEventConsumerThread.joinable()) {
			std::unique_lock lock(m_AsyncEventsMutex);
			m_AsyncEventsCV.notify_all();
			lock.unlock();
			m_AsyncEventConsumerThread.join();
		}
	}

	EventSubscriptionHandle::~EventSubscriptionHandle()
	{
		if (IsValid()) {
			m_EventManagerPtr->Unsubscribe(*this);
		}
	}

	EventSubscriptionHandle::EventSubscriptionHandle(EventSubscriptionHandle&& other) noexcept :
		m_SubscriptionListPtr(other.m_SubscriptionListPtr),
		m_Itr(other.m_Itr),
		m_Valid(other.m_Valid),
		m_EventManagerPtr(other.m_EventManagerPtr) {
		other.Invalidate();
	}

	EventSubscriptionHandle& EventSubscriptionHandle::operator=(EventSubscriptionHandle&& other) noexcept
	{
		if (&other != this) {
			if (IsValid()) {
				m_EventManagerPtr->Unsubscribe(*this);
			}
			m_EventManagerPtr = other.m_EventManagerPtr;
			m_SubscriptionListPtr = other.m_SubscriptionListPtr;
			m_Itr = other.m_Itr;
			m_Valid = other.m_Valid;
			other.Invalidate();
		}
		return *this;
	}
}