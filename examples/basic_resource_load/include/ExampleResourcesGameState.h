#pragma once
#include "Core.h"

class ExampleResourcesGameState : public Smasher::GameState {
	void Init() override;
	void Render(sf::RenderWindow& window) override;
};