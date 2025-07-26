#pragma once
#include "GameState.h"

class ExampleAppGameState : public Smasher::GameState {
public:
	ExampleAppGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {};
	void Reset() override;
	void Render(sf::RenderWindow& window) override;
};
