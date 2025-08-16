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
#include <SFML/Window.hpp>
#include "Engine.h"
#include "Exceptions.h"
#include "EngineConfig.h"
#include "EventFeeder.h"

namespace Smasher {

	Engine::Engine() :
		m_Headless(false),
		m_Window(
			std::make_unique<sf::RenderWindow>(
				sf::VideoMode(EngineConfig::WINDOW_WIDTH, EngineConfig::WINDOW_HEIGHT),
				EngineConfig::TITLE, sf::Style::Default,
				EngineConfig::DEFAULT_SETTINGS
			)
		) {
		Init();
	}

	Engine::Engine(bool headless) : m_Headless(true) {
		assert(headless == true);
		Init();
	}

	Engine::Engine(int width, int height, const sf::ContextSettings& settings) :
		m_Headless(false),
		m_Window(
			std::make_unique<sf::RenderWindow>(
				sf::VideoMode(EngineConfig::WINDOW_WIDTH, EngineConfig::WINDOW_HEIGHT),
				EngineConfig::TITLE,
				sf::Style::Default,
				settings
			)
		) {
		Init();
	}

	Engine::Engine(int width, int height) :
		m_Headless(false),
		m_Window(
			std::make_unique<sf::RenderWindow>(
				sf::VideoMode(width, height),
				EngineConfig::TITLE,
				sf::Style::Default, EngineConfig::DEFAULT_SETTINGS
			)
		) {
		Init();
	}

	Engine::~Engine()
	{
		m_GameStateByType.clear();
	}

	Engine::Engine(Engine&& other) noexcept : 
		m_Headless(other.m_Headless),
		m_GameStateByType(std::move(other.m_GameStateByType)),
		m_Window(std::move(other.m_Window)),
		m_IsWindowOpen(other.m_IsWindowOpen),
		m_EventManager(std::move(other.m_EventManager)),
		m_ResourceManager(std::move(other.m_ResourceManager)),
		m_WindowCloseHandle(std::move(other.m_WindowCloseHandle)),
		m_UpdateTimestamp(other.m_UpdateTimestamp),
		m_RenderTimestamp(other.m_RenderTimestamp)
	{
		bool tmp = other.m_RunningAtomic;
		m_RunningAtomic = tmp;
		m_UpdateInterval = EngineConfig::UPDATE_INTERVAL;
		m_RenderInterval = EngineConfig::RENDER_INTERVAL;
	}

	Engine Engine::CreateHeadless()
	{
		return Engine(true);
	}

	void Engine::Init() {
		glewExperimental = GL_TRUE;
		GLenum status = glewInit();
		if (status != GLEW_OK) {
			std::string_view errCode = reinterpret_cast<const char*>(glewGetErrorString(status));
			std::string message = std::format("GLEW Failed to initialize, status: {}", errCode);
			throw Exceptions::GLEWInitFailed(message);
		}

		m_WindowCloseHandle = m_EventManager.Subscribe<Events::WindowCloseEvent>(&Engine::OnWindowClose, this);
	}

	void Engine::Run() {
		EventFeeder m_EventFeeder(m_EventManager);
		try {
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			Millisecond updateTimer{ 0 };
			Millisecond renderTimer{ 0 };

			while (m_Window->isOpen() and m_RunningAtomic) {
				std::chrono::time_point<std::chrono::system_clock> tmp = std::chrono::system_clock::now();
				Millisecond diff = std::chrono::duration_cast<std::chrono::milliseconds>(tmp - now);
				now = tmp;
				sf::Event event;
				while (m_Window->pollEvent(event)) {
					m_EventFeeder.ForwardSFMLEvent(event);
				}

				updateTimer += diff;
				renderTimer += diff;

				m_EventManager.Dispatch();

				if (updateTimer >= m_UpdateInterval) {
					Update(Millisecond{ updateTimer });
					updateTimer = Millisecond{ 0 };
				}
				if (renderTimer >= m_RenderInterval) {
					Render(*m_Window);
					renderTimer = Millisecond{ 0 };
				}

#ifdef BENCHMARK
				BENCHMARK_LogAccumulatedTime();
#endif

				Millisecond minInterval = std::min(m_UpdateInterval, m_RenderInterval);
				std::chrono::milliseconds loopTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now);
				Millisecond sleepTime = std::max(minInterval - loopTime, std::chrono::milliseconds::zero());
				std::this_thread::sleep_for(sleepTime);
			}
			Shutdown();
		}
		catch (const std::exception& e) {
			std::cerr << "Exception Thrown: " << e.what() << std::endl;
			throw e;
		}
	}

	void Engine::Update(Millisecond delta) {
		for (auto& [_, pGameState] : m_GameStateByType) {
			m_UpdateTimestamp = std::chrono::system_clock::now();
			if (pGameState->GetStatus() == GameStateStatus::ACTIVE) {
				pGameState->PreUpdate(delta);
				pGameState->PreUpdateComponentManagers(delta);
				pGameState->Update(delta);
				pGameState->UpdateComponentManagers(delta);
			}
			Millisecond diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_UpdateTimestamp);
			pGameState->SetUpdateTime(diff);
		}
	}

	void Engine::Render(sf::RenderWindow& rWindow) {
		//rWindow.pushGLStates();
		m_Window->clear();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Screen And Depth Buffer
		for (const auto& [_, pGameState] : m_GameStateByType) {
			m_RenderTimestamp = std::chrono::system_clock::now();

			if (pGameState->GetStatus() == GameStateStatus::ACTIVE) {
				pGameState->Render(rWindow);
				pGameState->RenderComponentManagers(rWindow);
			}

			Millisecond diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_RenderTimestamp);
			pGameState->SetRenderTime(diff);
		}
		m_Window->display();
		//rWindow.popGLStates();

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

	void Engine::OnWindowClose(const Events::WindowCloseEvent& event) {
		m_RunningAtomic = false;
	}

	void Engine::Shutdown() {
		m_RunningAtomic = false;

		std::scoped_lock lock(m_WindowMutex);
		if (m_IsWindowOpen) {
			m_Window->close();
			m_IsWindowOpen = false;
		}
	}

	sf::RenderWindow& Engine::GetWindow() { return *m_Window; }
	EventManager& Engine::GetEventManager() { return m_EventManager; };
	ResourceManager& Engine::GetResourceManager() { return m_ResourceManager; };

}