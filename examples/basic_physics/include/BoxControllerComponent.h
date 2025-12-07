#include "Smasher/Base.h"
#include "Smasher/IComponent.h"

class BoxControllerComponent : public Smasher::IComponent {
public:
	BoxControllerComponent() = default;

	static void StaticUpdateComponent(BoxControllerComponent& self, const Smasher::Millisecond& delta);

	static inline const float VELOCITY = 600.f;
};