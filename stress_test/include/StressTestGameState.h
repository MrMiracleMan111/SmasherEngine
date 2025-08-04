#pragma once
#include <SFML/Window.hpp>
#include "Base.h"
#include "GameState.h"

class StressTestGameState : public Smasher::GameState {
public:
	void Update(Smasher::Millisecond delta) override;
	void Render(sf::RenderWindow& rWindow) override;
};