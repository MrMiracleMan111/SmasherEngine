#include <gtest/gtest.h>
#include "Core.h"
#include "BaseComponentManager.h"
#include "Components/CameraComponent.h"

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
	Smasher::Entity* pEntity = nullptr;
};

void EngineSetupFixture::SetUp() {
	pEngine = std::make_unique<Smasher::Engine>(640, 420);
	pState = &pEngine->AddState<DummyGameState>();
	pEntity = &pState->AddEntity<Smasher::Entity>();
}

void EngineSetupFixture::TearDown() {
	pEngine.reset();
}

class Transform2DComponentFixture : public EngineSetupFixture {};
class CameraComponentFixture : public EngineSetupFixture {};

// Transform2D Tests
TEST_F(Transform2DComponentFixture, AddTransformComponent) {
	EXPECT_NO_THROW({ pState->GetEntity(pEntity->GetUUID()); });
}

// Camera Component Tests
TEST_F(CameraComponentFixture, RenderTargetNotSet) {
	EXPECT_NO_THROW({ pState->GetEntity(pEntity->GetUUID()); });

	pEntity->AddComponent<Smasher::CameraComponent>();

	EXPECT_THROW({
		pEntity->GetComponent<Smasher::CameraComponent>().ApplyToTarget();
	}, Smasher::Exceptions::CameraTargetNotSet);
}

TEST_F(CameraComponentFixture, ApplyRenderTarget) {

	pEntity->AddComponent<Smasher::CameraComponent>();

	EXPECT_NE(&pEntity->GetComponent<Smasher::CameraComponent>().GetView(), &pEngine->GetWindow().getView());

	EXPECT_NO_THROW({
		pEntity->GetComponent<Smasher::CameraComponent>()
				.SetPosition(sf::Vector2f(10.0f, 15.0f))
				.SetRotation(Smasher::Degrees { 35 })
				.SetSize(sf::Vector2f(104.0f, 105.0f))
				.ApplyToTarget(pEngine->GetWindow());
	});

	EXPECT_EQ(pEntity->GetComponent<Smasher::CameraComponent>().GetPosition(), pEngine->GetWindow().getView().getCenter());
	EXPECT_EQ(pEntity->GetComponent<Smasher::CameraComponent>().GetRotation(), pEngine->GetWindow().getView().getRotation());
	EXPECT_EQ(pEntity->GetComponent<Smasher::CameraComponent>().GetSize(), pEngine->GetWindow().getView().getSize());
}