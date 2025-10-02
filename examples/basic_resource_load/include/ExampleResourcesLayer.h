#pragma once
#include "Core.h"

class ExampleResourcesLayer : public Smasher::Layer {
public:
	ExampleResourcesLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {}

	void Init() override;
	void Render(sf::RenderWindow& window) override;
};