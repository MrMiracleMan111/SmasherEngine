#pragma once
#include "Core.h"

class ExampleResourcesGameState : public Smasher::GameState {
	void Reset() override;
	void Render(sf::RenderWindow& window) override;
};