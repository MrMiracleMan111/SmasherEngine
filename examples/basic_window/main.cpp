#include "Engine.h"
#include "ExampleAppGameState.h"
int main() {
	Smasher::Engine engine(640, 420);
	ExampleAppGameState& state = engine.AddState<ExampleAppGameState>();
	state.Activate();
	engine.Run();
}