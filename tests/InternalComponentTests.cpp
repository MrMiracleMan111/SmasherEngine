#include <gtest/gtest.h>
#include "Core.h"
#include "BaseComponentManager.h"

class DummyGameState : public Smasher::GameState {
public:
	DummyGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}
};

TEST(TransformComponentTests, AddTransformComponent) {
	Smasher::Engine engine(640, 420);
	DummyGameState& state = engine.AddState<DummyGameState>();
	Smasher::Entity& entity = state.AddEntity<Smasher::Entity>();
	EXPECT_NO_THROW({ state.GetEntity(entity.GetUUID()); });
}
