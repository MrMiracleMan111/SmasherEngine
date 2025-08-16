#include <gtest/gtest.h>
#include "Core.h"
#include "BaseComponentManager.h"

class DummyGameState : public Smasher::GameState {
public:
	DummyGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}
};

class EngineSetupFixture : public ::testing::Test {
	void SetUp() override;
	void TearDown() override;
protected:
	std::unique_ptr<Smasher::Engine> pEngine;
	Smasher::GameState* pState = nullptr;
};

void EngineSetupFixture::SetUp() {
	pEngine = std::make_unique<Smasher::Engine>(640, 420);
	pState = &pEngine->AddState<DummyGameState>();
}

void EngineSetupFixture::TearDown() {
	pEngine.reset();
}

class Transform2DComponentFixture : public EngineSetupFixture {};

// Transform2D Tests
TEST_F(Transform2DComponentFixture, AddTransformComponent) {
	Smasher::Entity& entity = pState->AddEntity<Smasher::Entity>();
	EXPECT_NO_THROW({ pState->GetEntity(entity.GetUUID()); });
}

}
