#include "Core.h"

class TopLayer : public Smasher::Layer {
public:
	TopLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {};
	~TopLayer();

	void Init() override;
private:
};