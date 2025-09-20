#pragma once
#include "Base.h"
#include "GameState.h"

class SerializeGameState : public Smasher::GameState {
public:
	SerializeGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}

	void Init() override;
	void OnClose(const Smasher::Events::WindowCloseEvent& e);
	Smasher::Entity& SpawnBouncingBall(sf::Vector2i position);

private:
	Smasher::EventSubscriptionHandle m_OnCloseSubscription;
	std::vector<std::reference_wrapper<Smasher::Entity>> m_Balls;
};