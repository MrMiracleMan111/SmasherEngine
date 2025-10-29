#include "Smasher/Core.h"
#include "Smasher/Interpolation.h"
#include "Smasher/UI.h"

class MidLayer : public Smasher::Layer {
public:
	MidLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {};
	~MidLayer();

	void Init() override;
	void Update(Smasher::Millisecond delta) override;
private:
	Smasher::Entity* m_FPSTrackerPtr = nullptr;
	Smasher::Millisecond m_UpdateTimeSum;
	std::size_t m_UpdateTimeSampleCount = 0;
	double m_UpdateTimeAverage = 0.0;
	static const std::size_t s_SamplesPerAverage = 10;

	Smasher::EventSubscriptionHandle m_OnMouseClick;
	Smasher::EventSubscriptionHandle m_OnMouseMove;
	Smasher::Interpolated<sf::Vector2f> m_PanelPosition;
	Smasher::UIPanelComponent* m_UIPanelPtr = nullptr;
};