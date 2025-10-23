#pragma once
#include "Smasher/EventManager.h"
#include "Smasher/Layer.h"

class ExampleWindowLayer : public Smasher::Layer {
public:
	ExampleWindowLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {};
	~ExampleWindowLayer();
	void Init() override;
	void Reset() override;
	void Render(sf::RenderWindow& window) override;

private:
	void OnKeyPress(Smasher::Events::KeyboardEvent& e);
	Smasher::EventSubscriptionHandle m_KeyPressSubscription;
};
