#include "EventManager.h"
#include "Events.h"
#include "Engine.h"
#include "Layer.h"

namespace Smasher {
	EventManager::~EventManager() {
		StopAsync();
	}

	EventManager::EventManager(EventManager&& other) noexcept :
		m_EventSubscriptionsByType(std::move(other.m_EventSubscriptionsByType)),
		m_EventQueue(std::move(other.m_EventQueue)),
		m_AsyncRunning(false),
		m_EngineRef(other.m_EngineRef)
	{
		other.StopAsync();
		m_AsyncEventSubscriptionsByType = std::move(other.m_AsyncEventSubscriptionsByType);
		m_AsyncEventQueue = std::move(other.m_AsyncEventQueue);
		m_AsyncEventSwapQueue = std::move(other.m_AsyncEventSwapQueue);
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

			m_EngineRef = other.m_EngineRef;
			m_EventSubscriptionsByType = std::move(other.m_EventSubscriptionsByType);
			m_AsyncEventSubscriptionsByType = std::move(other.m_AsyncEventSubscriptionsByType);
			m_EventQueue = std::move(other.m_EventQueue);
			m_AsyncEventQueue = std::move(other.m_AsyncEventQueue);
			m_AsyncEventSwapQueue = std::move(other.m_AsyncEventSwapQueue);
			m_AsyncRunning = true;
			m_AsyncEventConsumerThread = std::thread(&EventManager::AsyncEventConsumer, this);
		}
		return *this;
	}

	void EventManager::Dispatch() {
		for (auto& pEvent : m_EventQueue) {
			auto itr = m_EngineRef.get().m_LayerStack.begin();
			while (pEvent->Propagate && itr != m_EngineRef.get().m_LayerStack.end()) {
				itr->second->ProcessEvent(*pEvent);
				itr++;
			}
		}
		m_EventQueue.clear();
	}

	void EventManager::DispatchAsync(Engine& engine) {
		for (auto& pEvent : m_AsyncEventQueue) {
			std::scoped_lock lock(m_AsyncSubscriptionsMutex);
			auto itr = engine.m_LayerStack.begin();
			while (pEvent->Propagate && itr != engine.m_LayerStack.end()) {
				itr->second->ProcessAsyncEvent(*pEvent);
				itr++;
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
			DispatchAsync(m_EngineRef.get());
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

	void EventSubscriptionHandle::Unsubscribe() {
		if (!IsValid()) {
			throw Exceptions::EventHandleInvalid("Handle is invalid");
		}
		Invalidate();

		std::scoped_lock lock(m_EventManagerPtr->GetAsyncSubscriptionsMutex());
		m_SubscriptionListPtr->erase(m_Itr);
	}

	EventSubscriptionHandle::~EventSubscriptionHandle()
	{
		if (IsValid()) {
			Unsubscribe();
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
				Unsubscribe();
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