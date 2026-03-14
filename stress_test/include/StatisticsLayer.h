#pragma once
#include <chrono>
#include <SFML/Window.hpp>
#include "Smasher/Core.h"
#include "Smasher/ECS.h"
#include "Smasher/Layer.h"

class StressTestLayer;

class StatisticsLayer : public Smasher::Layer {
public:
	StatisticsLayer(Smasher::Engine& engine, StressTestLayer& stressLayer) :
		Smasher::Layer(engine),
		m_StressLayer(stressLayer),
		m_UpdateTimeAverage(0.0), m_RenderTimeAverage(0.0),
		m_UpdateTimeSum(Smasher::Millisecond::zero()),
		m_RenderTimeSum(Smasher::Millisecond::zero()) {
		m_UpdateTimeSum = Smasher::Millisecond::zero();
		m_RenderTimeSum = Smasher::Millisecond::zero();

	}

	void Init() override;
	void Update(Smasher::Millisecond delta) override;
	void Render(sf::RenderWindow& window) override;

private:
	StressTestLayer& m_StressLayer;
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
};