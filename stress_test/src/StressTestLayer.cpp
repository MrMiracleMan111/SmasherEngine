#include <numbers>
#include <cmath>
#include "StressTestLayer.h"
#include "Manifest.h"
#include "Base.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"
#include "Components/DrawableComponent.h"
#include "Components/CameraComponent.h"
#include "Resources.h"
#include "BallComponent.h"
#include "Entity.h"

Smasher::Entity& StressTestLayer::SpawnBouncingBall(sf::Vector2i position)
{
	const float toRadian = (float)(180.0 / std::numbers::pi);
	int minSpeed = 100;
	int speedVariance = 100;
	float angle = (float)(rand() % 360);
	float speed = (float)(rand() % speedVariance + minSpeed);
	float tmpX = std::cos(angle * toRadian) * speed;
	float tmpY = std::sin(angle * toRadian) * speed;
	float depth = (float)(rand() % 100) / 100.0f;
	Smasher::Entity& image = AddEntity<Smasher::Entity>();
	image.AddComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(position.x, position.y))
		.SetScale(sf::Vector2f(20.0f, 20.0f))
		.SetDepth(depth)
		.GetEntity()
		.AddComponent<BallComponent>()
		.SetVelocity(sf::Vector2f(tmpX, tmpY));
	return image;
}

void StressTestLayer::Init()
{
	std::shared_ptr<Smasher::ShaderResource> pShader = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Resources::Shaders::basic_texture_shader, Smasher::ShaderResource>();
	sf::Vector2f windowSize = sf::Vector2f(GetEngine().GetWindow().getSize().x, GetEngine().GetWindow().getSize().y);
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(GetEngine().GetWindow().getView().getTransform().getMatrix());
	pShader->GetShader().setUniform("windowSize", windowSize);
	pShader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);

	m_OnMouseMoveHandle = GetEngine().GetEventManager().Subscribe<Smasher::Events::MouseMoveEvent>(&StressTestLayer::OnMouseMove, this);

	auto& rCompManager = static_cast<Smasher::DrawableComponentManager&>(GetComponentManager<Smasher::DrawableComponent>());
	rCompManager.SetShaderResource(pShader);

	Smasher::Entity& rUpdateTracker = AddEntity<Smasher::Entity>();
	Smasher::Entity& rRenderTracker = AddEntity<Smasher::Entity>();
	Smasher::Entity& rBallCounter = AddEntity<Smasher::Entity>();
	Smasher::Entity& rCamera = AddEntity<Smasher::Entity>();

	rUpdateTracker
		.AddComponent<Smasher::TextComponent>()
			.SetPosition(10.0f, 10.0f)
			.SetScale(1.0f, 1.0f)
			.UseDefaults()
			.SetFontAsset<Smasher::Resources::Fonts::arial>()
			.SetFillColor(sf::Color::White)
			.SetOutlineThickness(5.0f)
			.SetOutlineColor(sf::Color::Black);

	rRenderTracker
		.AddComponent<Smasher::TextComponent>()
			.SetPosition(10.0f, 50.0f)
			.SetScale(1.0f, 1.0f)
			.UseDefaults()
			.SetFontAsset<Smasher::Resources::Fonts::arial>()
			.SetFillColor(sf::Color::White)
			.SetOutlineThickness(5.0f)
			.SetOutlineColor(sf::Color::Black);

	rBallCounter
		.AddComponent<Smasher::TextComponent>()
			.SetPosition(10.0f, 100.0f)
			.SetScale(1.0f, 1.0f)
			.UseDefaults()
			.SetFontAsset<Smasher::Resources::Fonts::arial>()
			.SetFillColor(sf::Color::White)
			.SetOutlineThickness(5.0f)
			.SetOutlineColor(sf::Color::Black)
			.SetFontSize(20);

	// Add camera and apply it to window
	rCamera
		.AddComponent<Smasher::CameraComponent>()
			.SetSize(sf::Vector2f(GetEngine().GetWindow().getSize().x,
								  GetEngine().GetWindow().getSize().y))
			.SetTarget(GetEngine().GetWindow())
			.ApplyToTarget();

	m_UpdateTrackerPtr = &rUpdateTracker;
	m_RenderTrackerPtr = &rRenderTracker;
	m_BallCounterPtr = &rBallCounter;
	m_CameraPtr = &rCamera;

	// Create 10 entites
	// Half will have a non-alpha image
	// Half will have image with alpha pixels
	for (std::size_t i = 0; i < m_NumEntities; ++i) {
		float positionX = (float)((rand() % 100) * 5);
		float positionY = (float)((rand() % 100) * 5);
		Smasher::Entity& image = SpawnBouncingBall(sf::Vector2i(positionX, positionY));

		if (i % 2 == 0) {
			image.GetComponent<Smasher::DrawableComponent>()
				.SetTextureAsset<Smasher::Resources::Textures::alpha_test>({});
		}
		else {
			image.GetComponent<Smasher::DrawableComponent>()
				.SetTextureAsset<Smasher::Resources::Textures::small_art>({});
		}

		if (i == 10) {
			image.GetComponent<BallComponent>().SetCamera(rCamera.GetComponent<Smasher::CameraComponent>());
		}

	}
}

void StressTestLayer::Update(Smasher::Millisecond delta) {
	++m_UpdateTimeSampleCount;
	m_UpdateTimeSum += GetUpdateTime();
	if (m_UpdateTimeSampleCount >= s_SamplesPerAverage) {
		m_UpdateTimeAverage = (double)m_UpdateTimeSum.count() / (double)m_UpdateTimeSampleCount;
		m_UpdateTimeSampleCount = 0;
		m_UpdateTimeSum = Smasher::Millisecond::zero();
	}

	std::size_t numBalls = EntityCount();
	m_BallCounterPtr->GetComponent<Smasher::TextComponent>()
		.SetString(std::format("Number of Entites: {}", numBalls));

}

void StressTestLayer::Render(sf::RenderWindow& rWindow) {
	++m_RenderTimeSampleCount;
	Smasher::Millisecond tmp = GetRenderTime();
	m_RenderTimeSum += GetRenderTime();
	if (m_RenderTimeSampleCount >= s_SamplesPerAverage) {
		m_RenderTimeAverage = (double)m_RenderTimeSum.count() / (double)m_RenderTimeSampleCount;
		m_RenderTimeSampleCount = 0;
		m_RenderTimeSum = Smasher::Millisecond::zero();
	}

	m_UpdateTrackerPtr->GetComponent<Smasher::TextComponent>()
		.SetString(std::format("Update: {}ms", m_UpdateTimeAverage));
	
	m_RenderTrackerPtr->GetComponent<Smasher::TextComponent>()
		.SetString(std::format("Render: {}ms", m_RenderTimeAverage));
}

// Spawn an entity at mouse position
void StressTestLayer::OnMouseMove(Smasher::Events::MouseMoveEvent& event) {
	sf::Window& rWindow = GetEngine().GetWindow();
	sf::Vector2i entityPos = sf::Vector2i(event.Position) - rWindow.getPosition();
	Smasher::Entity& ball = SpawnBouncingBall(entityPos);
	ball.GetComponent<Smasher::DrawableComponent>()
		.SetTextureAsset<Smasher::Resources::Textures::alpha_test>({});
}
