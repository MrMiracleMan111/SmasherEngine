#pragma once
#include <chrono>
#include <SFML/Window.hpp>
#include "Base.h"
#include "GameState.h"

class StressTestGameState : public Smasher::GameState {
public:
	StressTestGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}
	void Init() override;
	void Update(Smasher::Millisecond delta) override;
	void Render(sf::RenderWindow& rWindow) override;

private:
	Smasher::Entity* m_UpdateTrackerPtr = nullptr;
	Smasher::Entity* m_RenderTrackerPtr = nullptr;
};