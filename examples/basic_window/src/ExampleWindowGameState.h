#pragma once
#include "GameState.h"

class ExampleWindowGameState : public Smasher::GameState {
public:
	ExampleWindowGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {};
	void Reset() override;
	void Render(sf::RenderWindow& window) override;
};
