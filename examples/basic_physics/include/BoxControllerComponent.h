#include "Smasher/Base.h"
#include "Smasher/IComponent.h"

class BoxControllerComponent : public Smasher::IComponent {
public:
	BoxControllerComponent() = default;

	static void StaticUpdateComponent(BoxControllerComponent& self, Smasher::Millisecond& delta);
	static void StaticRenderComponent(BoxControllerComponent& self, sf::RenderWindow& rWindow);

private:
	float velocity = 600.0f;
};