#include "Smasher/EventManager.h"
#include "Smasher/Events.h"
#include "Smasher/Engine.h"
#include "Smasher/Layer.h"

namespace Smasher {
	EventManager::~EventManager() {
		StopAsync();
	}

	EventManager::EventManager(EventManager&& other) noexcept :
		m_EventQueue(std::move(other.m_EventQueue)),
		m_AsyncRunning(false),
		m_EngineRef(other.m_EngineRef)
	{
		other.StopAsync();
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
			auto itr = m_EngineRef.get().m_LayerStack.rbegin();
			while (pEvent->CanPropagate() && itr != m_EngineRef.get().m_LayerStack.rend()) {
				itr->second->ProcessEvent(*pEvent);
				++itr;
			}
		}
		m_EventQueue.clear();
	}

	void EventManager::DispatchAsync(Engine& engine) {
		for (auto& pEvent : m_AsyncEventQueue) {
			// Layers and Asyc Subscriptions may change in between processing Async Events
			std::scoped_lock lock(m_AsyncSubscriptionsMutex, engine.m_LayerTransitionMutex);
			auto itr = engine.m_LayerStack.rbegin();
			while (pEvent->CanPropagate() && itr != engine.m_LayerStack.rend()) {
				itr->second->ProcessAsyncEvent(*pEvent);
				++itr;
			}
		}
		m_AsyncEventQueue.clear();
	}

	void EventManager::AsyncEventConsumer() {
		std::unique_lock lock(m_AsyncEventsMutex);
		while (m_AsyncRunning) {
			// Need to keep !m_AsyncRunning so that loop can shutdown when m_AsyncRunning = false
			m_AsyncEventsCV.wait(lock, [this] { return (m_AsyncEventSwapQueue.size() > 0 || !m_AsyncRunning); });
			m_AsyncEventSwapQueueMutex.lock();
			std::swap(m_AsyncEventSwapQueue, m_AsyncEventQueue);
			try {
				DispatchAsync(m_EngineRef.get());
			}
			catch (std::exception& e) {
				std::cerr << "Exception thrown: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "Unknown Exception thrown inside EventManager::DispatchAsync" << std::endl;
			}
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
		m_SubscriptionPtr(other.m_SubscriptionPtr),
		m_EventManagerPtr(other.m_EventManagerPtr),
		m_Itr(other.m_Itr),
		m_Valid(other.m_Valid) {
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
			m_SubscriptionPtr = other.m_SubscriptionPtr;
			m_Itr = other.m_Itr;
			m_Valid = other.m_Valid;
			other.Invalidate();
		}
		return *this;
	}
}