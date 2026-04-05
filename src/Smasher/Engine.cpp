#include <chrono>
#include <GL/glew.h>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif

#include <format>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <SDL3/SDL.h>
#include <SFML/Window.hpp>
#include "Smasher/Engine.h"
#include "Smasher/Exceptions.h"
#include "Smasher/EngineConfig.h"
#include "Smasher/EventFeeder.h"
#include "Smasher/ErrorCodes.h"
#include "Smasher/Layer.h"
#include "Smasher/ComponentSystems/EngineSystem.h"
#include "Smasher/ComponentSystems/SDLSystem.h"
#include "Smasher/Core.h"

namespace Smasher {
	Engine::Engine() :
		m_Headless(false),
		m_EventManager(*this),
		m_WindowPtr(
			//std::make_unique<sf::RenderWindow>(
			//	sf::VideoMode({ EngineConfig::WINDOW_WIDTH,EngineConfig::WINDOW_HEIGHT }),
			//	sf::String{ EngineConfig::TITLE },
			//	sf::State::Windowed,
			//	EngineConfig::DEFAULT_SETTINGS
			//)
		),
		m_WindowView(
			//sf::FloatRect{ { 0.f, 0.f }, { (float)m_WindowPtr->getSize().x, (float)m_WindowPtr->getSize().y } }
		)
	{
		Init();
	}

	Engine::Engine(bool headless) : m_Headless(true), m_WindowPtr(nullptr), m_EventManager(*this) {
		assert(headless == true);
		Init();
	}

	Engine::Engine(unsigned int width, unsigned int height, const sf::ContextSettings &settings) :
		m_Headless(false),
		m_EventManager(*this),
		m_WindowPtr(
			//std::make_unique<sf::RenderWindow>(
			//	sf::VideoMode({ EngineConfig::WINDOW_WIDTH, EngineConfig::WINDOW_HEIGHT }),
			//	sf::String{ EngineConfig::TITLE },
			//	sf::State::Windowed,
			//	settings
			//)
		),
		m_WindowView(
			//sf::FloatRect{ { 0.f, 0.f }, { (float)m_WindowPtr->getSize().x, (float)m_WindowPtr->getSize().y } }
		)
	{
		Init();
	}

	Engine::Engine(unsigned int width, unsigned int height) :
		m_Headless(false),
		m_EventManager(*this),
		m_WindowPtr(
			//std::make_unique<sf::RenderWindow>(
			//	sf::VideoMode({ width, height }),
			//	sf::String{ EngineConfig::TITLE },
			//	sf::State::Windowed,
			//	EngineConfig::DEFAULT_SETTINGS
			//)
		),
		m_WindowView(
			//sf::FloatRect{ { 0.f, 0.f }, { (float)m_WindowPtr->getSize().x, (float)m_WindowPtr->getSize().y } }
		)
	{
		Init();
	}

	Engine::~Engine()
	{
		// Clear up any last async jobs
		m_JobManager.WaitForAsyncJobs();
		m_EventManager.StopAsync();
		if (m_Valid) {
			if (!m_Headless) {
				m_WindowCloseHandle.Unsubscribe(); // Explicilty unsubscribe before EventManager is deconstructed
				m_WindowResizeHandle.Unsubscribe();
			}
			else {
			}
			m_LayerStack.clear();
		}
		SDL_Quit();
	}

	Engine::Engine(Engine &&other) noexcept : 
		m_Headless(other.m_Headless),
		m_LayerStack(std::move(other.m_LayerStack)),
		m_WindowPtr(std::move(other.m_WindowPtr)),
		m_IsWindowOpen(other.m_IsWindowOpen),
		m_EventManager(std::move(other.m_EventManager)),
		m_ResourceManager(std::move(other.m_ResourceManager)),
		m_WindowCloseHandle(std::move(other.m_WindowCloseHandle)),
		m_WindowResizeHandle(std::move(other.m_WindowResizeHandle)),
		m_UpdateTimestamp(other.m_UpdateTimestamp),
		m_RenderTimestamp(other.m_RenderTimestamp),
		m_UpdateInterval(other.m_UpdateInterval),
		m_RenderInterval(other.m_RenderInterval)
	{
		bool tmp = other.m_RunningAtomic;
		m_RunningAtomic = tmp;
		other.m_Valid = false;
	}

	Engine& Engine::operator =(Engine &&other) noexcept
	{
		if (&other != this) {
			m_Headless = other.m_Headless;
			m_LayerStack = std::move(other.m_LayerStack);
			m_WindowPtr = std::move(other.m_WindowPtr);
			m_IsWindowOpen = other.m_IsWindowOpen;
			m_EventManager = std::move(other.m_EventManager);
			m_ResourceManager = std::move(other.m_ResourceManager);
			m_WindowCloseHandle = std::move(other.m_WindowCloseHandle);
			m_UpdateTimestamp = other.m_UpdateTimestamp;
			m_RenderTimestamp = other.m_RenderTimestamp;
			m_UpdateInterval = other.m_UpdateInterval;
			m_RenderInterval = other.m_RenderInterval;
			bool tmp = other.m_RunningAtomic;
			m_RunningAtomic = tmp;
			other.m_Valid = false;
		}
		return *this;
	}

	Engine Engine::CreateHeadless()
	{
		return Engine(true);
	}

	void Engine::Init() {
		BaseLayer &base = PushLayer<BaseLayer>(); // Base layer

#if defined(_WIN32)
		timeBeginPeriod(1);
#elif defined(__linux__)
#elif defined(__APPLE__)
#endif

		if (!m_Headless) {
			SDL_Init(SDL_INIT_VIDEO);

			//glewExperimental = GL_TRUE;
			//GLenum status = glewInit();
			//if (status != GLEW_OK) {
			//	std::string_view errCode = reinterpret_cast<const char*>(glewGetErrorString(status));
			//	std::string message = std::format("GLEW Failed to initialize, status: {}", errCode);
			//	throw Exceptions::GLEWInitFailed(message);
			//}

			m_WindowCloseHandle = base.Subscribe<Events::WindowCloseEvent>(&Engine::OnWindowClose, this);
			m_WindowResizeHandle = base.Subscribe<Events::WindowResizeEvent>(&Engine::OnWindowResize, this);
		}
		else {
			SDL_Init(0);
		}

		m_Registry.ctx().emplace<EngineSystem::Context>(*this);
	}

	void Engine::Run() {
		m_RunningAtomic = true;

#ifdef CATCH_EXCEPTIONS
		try {
#endif
			std::chrono::time_point<std::chrono::system_clock> lastTickTime = std::chrono::system_clock::now();
			std::chrono::microseconds updateTimer{ 0 };
			std::chrono::microseconds renderTimer{ 0 };

			while ((!m_Headless) && m_RunningAtomic) {
				assert(m_Registry.ctx().contains<SDLSystem::Context>() && "SDLSystem was not initialized");

				std::chrono::time_point<std::chrono::system_clock> tmp = std::chrono::system_clock::now();
				// Time passed since last tick
				m_TickDelta = std::chrono::duration_cast<std::chrono::microseconds>(tmp - lastTickTime);
				lastTickTime = tmp;
				if (!m_Headless) {
					auto& sdlCtx = m_Registry.ctx().get<SDLSystem::Context>();
					SDL_Event event;
					while (SDL_PollEvent(&event)) {
						EventFeeder::ForwardSDLEvent(*this, event);
					}
				}

				updateTimer += m_TickDelta;
				renderTimer += m_TickDelta;

				m_IsRenderTick = (!m_Headless && renderTimer >= m_RenderInterval);
				m_EventManager.Dispatch();

				if (updateTimer >= m_UpdateInterval) {
					Update(std::chrono::duration_cast<std::chrono::milliseconds>( updateTimer ));
					updateTimer = std::chrono::microseconds{ 0 };
				}

				// Reset render timer on render tick
				renderTimer = m_IsRenderTick ? std::chrono::microseconds{ 0 } : renderTimer;


#ifdef BENCHMARK
				BENCHMARK_LogAccumulatedTime();
#endif

				Millisecond minInterval = std::min(m_UpdateInterval, m_RenderInterval);
				std::chrono::milliseconds currentTickTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastTickTime);
				Millisecond sleepTime = std::max(minInterval - currentTickTime, std::chrono::milliseconds::zero());

				m_IsRenderTick = false;
				PrecisionSleep(sleepTime);
			}
			Shutdown();
		}
#ifdef CATCH_EXCEPTIONS
		catch (const std::exception &e) {
			std::cerr << "Exception Thrown: " << e.what() << std::endl;
			throw e;
		}
	}
#endif


	void Engine::HandleLayerTransitions() {
		for (auto &itr : m_LayerTransitions) {
			switch (itr.type) {
				case LayerTransitionType::ADD:
					AddLayer(itr);
					break;
				case LayerTransitionType::REMOVE:
					RemoveLayer(itr);
					break;
				default:
					//Error
					assert(false);
			}
		}
		m_LayerTransitions.clear();
	}

	Engine::LayerStackConstItr Engine::GetLayerItr(std::type_index index) const
	{
		for (auto itr = m_LayerStack.begin(); itr != m_LayerStack.end(); ++itr) {
			if (index == itr->first) {
				return itr;
			}
		}
		return m_LayerStack.end();
	}

	// Updates layers from bottom to top
	void Engine::Update(Millisecond delta) {

		if (m_PhysicsManager.IsInitialized()) {
			m_PhysicsManager.Step(delta);
		}

		HandleLayerTransitions();

		m_JobManager.RunTickJobProducer();

		for (auto &itr : m_LayerStack) {
			std::unique_ptr<Layer> &pLayer = itr.second;
			m_UpdateTimestamp = std::chrono::system_clock::now();
			if (pLayer->GetStatus() == LayerStatus::ACTIVE) {
				pLayer->PreUpdate(delta);
				pLayer->PreUpdateComponentManagers(delta);
				pLayer->Update(delta);
				pLayer->UpdateComponentManagers(delta);
				pLayer->PostUpdate(delta);
				pLayer->PostUpdateComponentManagers(delta);
			}
			Millisecond diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_UpdateTimestamp);
			pLayer->SetUpdateTime(diff);
		}

		m_JobManager.RunJobs();

		m_JobManager.WaitForAsyncJobs();
	}

	ErrorCode Engine::DisplayWindow() {
		if (m_Headless) {
			return ERROR_EngineIsInHeadlessMode;
		}

		m_WindowPtr->display();

		return ERROR_NoError;
	}

	ErrorCode Engine::ClearWindow(sf::Color color) {
		if (m_Headless) {
			return ERROR_EngineIsInHeadlessMode;
		}

		m_WindowPtr->clear(color);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Screen And Depth Buffer

		return ERROR_NoError;
	}

	inline std::chrono::microseconds Engine::GetTickDelta() {
		return m_TickDelta;
	}

	// Should rendering occur on this tick (determiend by render interval)
	inline bool Engine::IsRenderTick() {
		return m_IsRenderTick;
	}


	// Renders layers bottom to top order (so that top layer appears above other layers)
	void Engine::Render() {
		m_WindowPtr->clear();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Screen And Depth Buffer

		// Render in reverse order
		for (auto &itr : m_LayerStack) {
			std::unique_ptr<Layer> &pLayer = itr.second;
			m_RenderTimestamp = std::chrono::system_clock::now();

			if (pLayer->GetStatus() == LayerStatus::ACTIVE) {
				pLayer->Render(*m_WindowPtr);
				pLayer->RenderComponentManagers(*m_WindowPtr);
			}

			Millisecond diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_RenderTimestamp);
			pLayer->SetRenderTime(diff);
		}

		m_WindowPtr->display();

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			std::cout << "GL Error: \"" << gluErrorString(err) << "\" Code: " << err << std::endl;
		}
	}

#ifdef	BENCHMARK
	void Engine::BENCHMARK_LogAccumulatedTime() {
		// Calculate average update time in ms after every 10 samples
		InternalTimers::SMASHER_TimeSampleSum += InternalTimers::SMASHER_TimeAccumulator;
		InternalTimers::SMASHER_TimeAccumulator = std::chrono::microseconds::zero();
		++InternalTimers::SMASHER_TimeSampleCount;
		if (InternalTimers::SMASHER_TimeSampleCount % 10 == 0) {
			double avg = (double)(InternalTimers::SMASHER_TimeSampleSum.count()) / (double)InternalTimers::SMASHER_TimeSampleCount;
			InternalTimers::SMASHER_TimeSampleCount = 0;
			InternalTimers::SMASHER_TimeSampleSum = std::chrono::microseconds::zero();
			std::printf("Accumulated Time: %.2fms\n", avg / 1000.0);
		}
	}
#endif

	void Engine::OnWindowClose(Events::WindowCloseEvent &event) {
		m_RunningAtomic = false;
	}

	void Engine::OnWindowResize(Events::WindowResizeEvent &event) {
		//m_WindowView.setSize(sf::Vector2f{ (float)event.WindowSize.x, (float)event.WindowSize.y });
		//m_WindowPtr->setView(m_WindowView);
	}


	void Engine::AddLayer(LayerTransition &transition)
	{
		std::scoped_lock lock(m_LayerTransitionMutex);
		Layer &layer = *transition.pLayer;
		std::type_index index = transition.addTransition.index;
		m_LayerStack.emplace_back(index, std::move(transition.pLayer));
		layer.Init();
	}

	void Engine::RemoveLayer(LayerTransition &transition)
	{
		std::scoped_lock lock(m_LayerTransitionMutex);
		if (transition.removeTransition.index == std::type_index(typeid(BaseLayer))) {
			throw Exceptions::CannotRemoveBaseLayer("Removing BaseLayer is not permitted.");
		}

		auto itr = GetLayerItr(transition.removeTransition.index);
		bool hasLayer = (itr != m_LayerStack.end());

		if (!hasLayer) {
			throw Exceptions::LayerNotFound("Could not find layer to remove");
		}

		m_LayerStack.erase(itr);
	}

	void Engine::Shutdown() {
		m_RunningAtomic = false;

		std::scoped_lock lock(m_WindowMutex);
		if (m_IsWindowOpen) {
			m_WindowPtr->close();
			m_IsWindowOpen = false;
		}

#if defined(_WIN32)
		timeEndPeriod(1);
#elif defined(__linux__)
#elif defined(__APPLE__)
#endif
	}

	Engine::LayerStackItr Engine::TopLayerItr()
	{
		return m_LayerStack.begin();
	}

	sf::RenderWindow& Engine::GetWindow() { return *m_WindowPtr; }
	EventManager& Engine::GetEventManager() { return m_EventManager; };
	ResourceManager& Engine::GetResourceManager() { return m_ResourceManager; };
	PhysicsManager& Engine::GetPhysicsManager() { return m_PhysicsManager; };
	JobManager& Engine::GetJobManager() { return m_JobManager; };
	entt::registry& Engine::GetRegistry() { return m_Registry; };

}