#pragma once
#include "GameState.h"

class ExampleAppGameState : public Smasher::GameState {
public:
	using Smasher::GameState::GameState;
	void Reset() override;
	void Render(sf::RenderWindow& window) override;
};
