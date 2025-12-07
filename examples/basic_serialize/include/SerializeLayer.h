#pragma once
#include "Smasher/Base.h"
#include "Smasher/Layer.h"

class SerializeLayer : public Smasher::Layer {
public:
	SerializeLayer(Smasher::Engine &engine) : Smasher::Layer(engine) {}

	void Init() override;
	void OnClose(Smasher::Events::WindowCloseEvent &event);
	Smasher::Entity &SpawnBouncingBall(sf::Vector2i position);

private:
	Smasher::EventSubscriptionHandle m_OnCloseSubscription;
	std::vector<std::reference_wrapper<Smasher::Entity>> m_Balls;
};