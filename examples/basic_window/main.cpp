#include "Engine.h"
#include "ExampleAppGameState.h"
int main() {
	Smasher::Engine engine(640, 420);
	ExampleAppGameState state(engine);
	engine.AddState(1, state);
	engine.Run();
}