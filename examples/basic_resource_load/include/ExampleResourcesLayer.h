#pragma once
#include "Core.h"

class ExampleResourcesGameState : public Smasher::Layer {
public:
	ExampleResourcesGameState(Smasher::Engine& engine) : Smasher::Layer(engine) {}

	void Init() override;
	void Render(sf::RenderWindow& window) override;
};