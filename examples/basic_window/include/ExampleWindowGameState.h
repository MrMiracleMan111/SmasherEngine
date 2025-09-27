#pragma once
#include "EventManager.h"
#include "GameState.h"

class ExampleWindowGameState : public Smasher::GameState {
public:
	ExampleWindowGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {};
	~ExampleWindowGameState();
	void Init() override;
	void Reset() override;
	void Render(sf::RenderWindow& window) override;

private:
	void OnKeyPress(const Smasher::Events::KeyboardEvent& e);
	Smasher::EventSubscriptionHandle m_KeyPressSubscription;
};
