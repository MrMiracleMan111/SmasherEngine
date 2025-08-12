#pragma once
#include <chrono>
#include <SFML/Window.hpp>
#include "Base.h"
#include <chrono>
#include "GameState.h"

class StressTestGameState : public Smasher::GameState {
public:
	StressTestGameState(Smasher::Engine& engine, std::size_t numEntities) :
		Smasher::GameState(engine),
		m_NumEntities(numEntities),
		m_UpdateTimeAverage(0.0), m_RenderTimeAverage(0.0), 
		m_UpdateTimeSum(Smasher::Millisecond::zero()),
		m_RenderTimeSum(Smasher::Millisecond::zero()) {
			m_UpdateTimeSum = Smasher::Millisecond::zero();
			m_RenderTimeSum = Smasher::Millisecond::zero();
	
	}
	void Init() override;
	void Update(Smasher::Millisecond delta) override;
	void Render(sf::RenderWindow& rWindow) override;

private:
	void OnMouseMove(const Smasher::Events::MouseMoveEvent& event);
	Smasher::Entity& SpawnBouncingBall(sf::Vector2i position);

	Smasher::Millisecond m_UpdateTimeSum;
	Smasher::Millisecond m_RenderTimeSum;
	double m_UpdateTimeAverage = 0.0;
	double m_RenderTimeAverage = 0.0;
	std::size_t m_UpdateTimeSampleCount = 0;
	std::size_t m_RenderTimeSampleCount = 0;
	static const std::size_t s_SamplesPerAverage = 10;
	Smasher::Entity* m_UpdateTrackerPtr = nullptr;
	Smasher::Entity* m_RenderTrackerPtr = nullptr;
	Smasher::Entity* m_BallCounterPtr = nullptr;
	Smasher::Entity* m_CameraPtr = nullptr;
	const std::size_t m_NumEntities;
	Smasher::EventSubscriptionHandle m_OnMouseMoveHandle;
};