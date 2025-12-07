#include "Smasher/Core.h"

class BottomLayer : public Smasher::Layer {
public:
	BottomLayer(Smasher::Engine &engine) : Smasher::Layer(engine) {};
	~BottomLayer();

	void Init() override;
	//void Update(Smasher::Millisecond delta) override { std::cout << "----\nBottom" << std::endl; }
private:
};