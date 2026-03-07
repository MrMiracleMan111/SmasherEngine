#pragma once
#include "Smasher/EventManager.h"
#include "Smasher/Events.h"
#include "Smasher/Layer.h"

class ExamplePhysicsLayer : public Smasher::Layer {
public:
	ExamplePhysicsLayer(Smasher::Engine &engine) : Smasher::Layer(engine) {};
	~ExamplePhysicsLayer();
	void Init() override;
	void Reset() override;
	void Render(sf::RenderWindow &window) override;

private:
	void OnKeyPress(Smasher::Events::KeyboardEvent &event);
	Smasher::EventSubscriptionHandle m_KeyPressSubscription;
	Smasher::EventSubscriptionHandle m_MousePressSubscription;
	Smasher::EventSubscriptionHandle m_MouseMoveSubscription;
};
