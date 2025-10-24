#include "Smasher/Core.h"

class TopLayer : public Smasher::Layer {
public:
	TopLayer(Smasher::Engine& engine) : Smasher::Layer(engine) {};
	~TopLayer();

	void Init() override;
	//void Update(Smasher::Millisecond delta) override { std::cout << "Top\n----" << std::endl; }
private:
};