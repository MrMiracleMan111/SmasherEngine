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
};

void EngineSetupFixture::SetUp() {
	pEngine = std::make_unique<Smasher::Engine>(640, 420);
	pState = &pEngine->AddState<DummyGameState>();
}

void EngineSetupFixture::TearDown() {
	pEngine.reset();
}

class Transform2DComponentFixture : public EngineSetupFixture {};
class CameraComponentFixture : public EngineSetupFixture {};

// Transform2D Tests
TEST_F(Transform2DComponentFixture, AddTransformComponent) {
	Smasher::Entity& entity = pState->AddEntity<Smasher::Entity>();
	EXPECT_NO_THROW({ pState->GetEntity(entity.GetUUID()); });
}

// Camera Component Tests
TEST_F(CameraComponentFixture, RenderTargetNotSet) {
	Smasher::Entity& entity = pState->AddEntity<Smasher::Entity>();
	EXPECT_NO_THROW({ pState->GetEntity(entity.GetUUID()); });

	entity.AddComponent<Smasher::CameraComponent>();

	EXPECT_THROW({
		entity.GetComponent<Smasher::CameraComponent>().ApplyToTarget();
	}, Smasher::Exceptions::CameraTargetNotSet);
}

TEST_F(CameraComponentFixture, ApplyRenderTarget) {
	Smasher::Entity& entity = pState->AddEntity<Smasher::Entity>();

	entity.AddComponent<Smasher::CameraComponent>();

	EXPECT_NE(&entity.GetComponent<Smasher::CameraComponent>().GetView(), &pEngine->GetWindow().getView());

	EXPECT_NO_THROW({
		entity.GetComponent<Smasher::CameraComponent>()
				.SetPosition(sf::Vector2f(10.0f, 15.0f))
				.SetRotation(Smasher::Degrees { 35 })
				.SetSize(sf::Vector2f(104.0f, 105.0f))
				.ApplyToTarget(pEngine->GetWindow());
	});

	EXPECT_EQ(entity.GetComponent<Smasher::CameraComponent>().GetPosition(), pEngine->GetWindow().getView().getCenter());
	EXPECT_EQ(entity.GetComponent<Smasher::CameraComponent>().GetRotation(), pEngine->GetWindow().getView().getRotation());
	EXPECT_EQ(entity.GetComponent<Smasher::CameraComponent>().GetSize(), pEngine->GetWindow().getView().getSize());
}