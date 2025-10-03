#include "Core.h"

class MidLayer : public Smasher::Layer {
public:
	MidLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {};
	~MidLayer();

	void Init() override;
private:
};