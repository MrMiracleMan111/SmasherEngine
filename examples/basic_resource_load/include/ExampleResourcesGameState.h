#pragma once
#include "Core.h"

class ExampleResourcesGameState : public Smasher::GameState {
public:
	ExampleResourcesGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}

	void Init() override;
	void Render(sf::RenderWindow& window) override;
};