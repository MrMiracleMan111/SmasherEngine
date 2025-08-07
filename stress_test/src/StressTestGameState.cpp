#include "StressTestGameState.h"
#include "Manifest.h"
#include "Base.h"
#include "Components/Transform2DComponent.h"
#include "Components/TextComponent.h"
#include "ComponentManagers/DrawableComponentManager.h"
#include "Components/DrawableComponent.h"
#include "Resources.h"
#include "BallComponent.h"

void StressTestGameState::Init()
{
	std::shared_ptr<Smasher::ShaderResource> shader = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Resources::Shaders::basic_texture_shader, Smasher::ShaderResource>();
	sf::Vector2f windowSize = sf::Vector2f(GetEngine().GetWindow().getSize().x, GetEngine().GetWindow().getSize().y);
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(GetEngine().GetWindow().getView().getTransform().getMatrix());
	shader->GetShader().setUniform("windowSize", windowSize);
	shader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);

	auto& rCompManager = static_cast<Smasher::DrawableComponentManager&>(GetComponentManager<Smasher::DrawableComponent>());
	rCompManager.SetShaderResource(shader);

	Smasher::Entity& rUpdateTracker = AddEntity<Smasher::Entity>();
	Smasher::Entity& rRenderTracker = AddEntity<Smasher::Entity>();

	rUpdateTracker
		.AddComponent<Smasher::Transform2DComponent>()
			.SetPosition(10.0f, 10.0f)
			.SetScale(1.0f, 1.0f)
			.GetEntity()
		.AddComponent<Smasher::TextComponent>()
			.UseDefaults()
			.SetFontAsset<Smasher::Resources::Fonts::arial>()
			.SetFillColor(sf::Color::White)
			.SetOutlineThickness(5.0f)
			.SetOutlineColor(sf::Color::Black);

	rRenderTracker
		.AddComponent<Smasher::Transform2DComponent>()
			.SetPosition(10.0f, 50.0f)
			.SetScale(1.0f, 1.0f)
			.GetEntity()
		.AddComponent<Smasher::TextComponent>()
			.UseDefaults()
			.SetFontAsset<Smasher::Resources::Fonts::arial>()
			.SetFillColor(sf::Color::White)
			.SetOutlineThickness(5.0f)
			.SetOutlineColor(sf::Color::Black);

	m_UpdateTrackerPtr = &rUpdateTracker;
	m_RenderTrackerPtr = &rRenderTracker;
}

void StressTestGameState::Update(Smasher::Millisecond delta) {
	++m_UpdateTimeSampleCount;
	m_UpdateTimeSum += GetUpdateTime();
	if (m_UpdateTimeSampleCount >= s_SamplesPerAverage) {
		m_UpdateTimeAverage = (double)m_UpdateTimeSum.count() / (double)m_UpdateTimeSampleCount;
		m_UpdateTimeSampleCount = 0;
		m_UpdateTimeSum = Smasher::Millisecond::zero();
	}
}

void StressTestGameState::Render(sf::RenderWindow& rWindow) {
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
