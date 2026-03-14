#include "StatisticsLayer.h"
#include "StressTestLayer.h"
#include "Smasher/Components/TextComponent.h"
#include "Manifest.h"

void StatisticsLayer::Init()
{
	Smasher::Entity& rUpdateTracker = AddEntity<Smasher::Entity>();
	Smasher::Entity& rRenderTracker = AddEntity<Smasher::Entity>();
	Smasher::Entity& rBallCounter = AddEntity<Smasher::Entity>();

	rUpdateTracker
		.AddComponent<Smasher::TextComponent>()
		.SetPosition(10.0f, 10.0f)
		.SetScale(1.0f, 1.0f)
		.UseDefaults()
		.SetFontAsset<Smasher::Manifest::Fonts::arial>()
		.SetFillColor(sf::Color::White)
		.SetOutlineThickness(5.0f)
		.SetOutlineColor(sf::Color::Black);

	rRenderTracker
		.AddComponent<Smasher::TextComponent>()
		.SetPosition(10.0f, 50.0f)
		.SetScale(1.0f, 1.0f)
		.UseDefaults()
		.SetFontAsset<Smasher::Manifest::Fonts::arial>()
		.SetFillColor(sf::Color::White)
		.SetOutlineThickness(5.0f)
		.SetOutlineColor(sf::Color::Black);

	rBallCounter
		.AddComponent<Smasher::TextComponent>()
		.SetPosition(10.0f, 100.0f)
		.SetScale(1.0f, 1.0f)
		.UseDefaults()
		.SetFontAsset<Smasher::Manifest::Fonts::arial>()
		.SetFillColor(sf::Color::White)
		.SetOutlineThickness(5.0f)
		.SetOutlineColor(sf::Color::Black)
		.SetFontSize(20);

	m_UpdateTrackerPtr = &rUpdateTracker;
	m_RenderTrackerPtr = &rRenderTracker;
	m_BallCounterPtr = &rBallCounter;
}

void StatisticsLayer::Update(Smasher::Millisecond delta) {
	++m_UpdateTimeSampleCount;
	m_UpdateTimeSum += m_StressLayer.GetUpdateTime();
	if (m_UpdateTimeSampleCount >= s_SamplesPerAverage) {
		m_UpdateTimeAverage = (double)m_UpdateTimeSum.count() / (double)m_UpdateTimeSampleCount;
		m_UpdateTimeSampleCount = 0;
		m_UpdateTimeSum = Smasher::Millisecond::zero();
	}

	std::size_t numBalls = m_StressLayer.EntityCount();
	m_BallCounterPtr->GetComponent<Smasher::TextComponent>()
		.SetString(std::format("Number of Entites: {}", numBalls));

}

void StatisticsLayer::Render(sf::RenderWindow& window) {
	++m_RenderTimeSampleCount;
	m_RenderTimeSum += m_StressLayer.GetRenderTime();
	if (m_RenderTimeSampleCount >= s_SamplesPerAverage) {
		m_RenderTimeAverage = (double)m_RenderTimeSum.count() / (double)m_RenderTimeSampleCount;
		m_RenderTimeSampleCount = 0;
		m_RenderTimeSum = Smasher::Millisecond::zero();
	}

	m_UpdateTrackerPtr->GetComponent<Smasher::TextComponent>()
		.SetString(std::format("Update: {}ms", m_UpdateTimeAverage));

	m_RenderTrackerPtr->GetComponent<Smasher::TextComponent>()
		.SetString(std::format("Render: {}ms", m_RenderTimeAverage));
}
