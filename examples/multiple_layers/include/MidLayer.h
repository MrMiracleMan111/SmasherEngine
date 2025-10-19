#include "Core.h"

class MidLayer : public Smasher::Layer {
public:
	MidLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {};
	~MidLayer();

	void Init() override;
	//void Update(Smasher::Millisecond delta) override { std::cout << "Middle" << std::endl; }
private:
};