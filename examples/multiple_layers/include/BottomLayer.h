#include "Core.h"

class BottomLayer : public Smasher::Layer {
public:
	BottomLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {};
	~BottomLayer();

	void Init() override;
private:
};