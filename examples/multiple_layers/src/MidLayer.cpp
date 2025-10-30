#include "MidLayer.h"
#include "Smasher/Components/TextComponent.h"
#include "Manifest.h"
#include "Smasher/UI.h"
#include "Smasher/Interpolation.h"

MidLayer::~MidLayer() {

}

void MidLayer::Update(Smasher::Millisecond delta) {
	if (m_UIPanelPtr != nullptr) {
		m_UIPanelPtr->SetPosition(m_PanelPosition.get());
	}

	++m_UpdateTimeSampleCount;
	m_UpdateTimeSum += GetUpdateTime();
	if (m_UpdateTimeSampleCount >= s_SamplesPerAverage) {
		m_UpdateTimeAverage = (double)m_UpdateTimeSum.count() / (double)m_UpdateTimeSampleCount;
		m_UpdateTimeSampleCount = 0;
		m_UpdateTimeSum = Smasher::Millisecond::zero();
	}

	m_FPSTrackerPtr->GetComponent<Smasher::TextComponent>()
		.SetString(std::format("Update: {}ms", m_UpdateTimeAverage));
}

void MidLayer::Init()
{
	m_PanelPosition.useCurve(Smasher::InterpolationType::EASE_OUT);
	m_PanelPosition.SetDuration(Smasher::Millisecond{ 1000 });
	std::shared_ptr<Smasher::ShaderResource> pShader = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Manifest::Shaders::basic_ui_shader, Smasher::ShaderResource>();
	Smasher::UIPanelComponentManager& rUIManager = static_cast<Smasher::UIPanelComponentManager&>(GetComponentManager<Smasher::UIPanelComponent>());
	sf::Vector2f windowSize = sf::Vector2f((float)GetEngine().GetWindow().getSize().x, (float)GetEngine().GetWindow().getSize().y);
	sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(GetEngine().GetWindow().getView().getTransform().getMatrix());
	pShader->GetShader().setUniform("windowSize", windowSize);
	pShader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);
	rUIManager.SetShaderResource(pShader);

	Smasher::Entity& entity = AddEntity();
	Smasher::Entity& fpsTracker = AddEntity();

	fpsTracker.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("FPS: ")
		.SetFillColor(sf::Color::White)
		.SetPosition(10.f, 10.f)
		.SetFontAsset<Smasher::Manifest::Fonts::arial>();

	m_FPSTrackerPtr = &fpsTracker;

	entity.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("Mid Layer")
		.SetFillColor(sf::Color::Magenta)
		.SetPosition(130.0f, 200.0f)
		.SetFontAsset<Smasher::Manifest::Fonts::arial>();

	Smasher::Entity& uiPanel = AddEntity();

	std::shared_ptr<Smasher::TextureResource> pTexture = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Manifest::Textures::small_art, Smasher::TextureResource>();

	Smasher::UIPanelComponent& panel = uiPanel.AddComponent<Smasher::UIPanelComponent>()
		.SetColor(sf::Color::Yellow)
		.SetBorderRadius(Smasher::UIPanelCorner::LEFT, 40.0f)
		.SetBorderColor(sf::Color::Red)
		.SetTexture(pTexture)
		.SetBorderThickness(10.0f)
		.SetPosition(100.0f, 60.0f)
		.SetScale(100.0f, 200.0f)
		//.SetClipRect(sf::IntRect{0, 0, 100, 100})
		//.SetClipRotation(Smasher::Degrees{ 10.0f })
		.SetDepth(0.4f);
	m_UIPanelPtr = &panel;
	panel.SetOnHoverCallback([&panel](Smasher::Events::MouseMoveEvent& event) {
		if ((bool)(panel.GetPanelState() & Smasher::UIPanelState::HOVERED)) {
			sf::Color tmp = sf::Color::Cyan;
			tmp.a = 40;
			panel.SetColor(tmp);
		}
		else {
			sf::Color tmp = sf::Color::Magenta;
			tmp.a = 40;
			panel.SetColor(tmp);
		}
	});

	m_OnMouseMove = Subscribe<Smasher::Events::MouseMoveEvent>([&panel](Smasher::Events::MouseMoveEvent& event) {
		//panel.SetPosition(sf::Vector2f(event.Position));
		panel.SetClipRotation(panel.GetClipRotation() + 1.0f);
		panel.SetRotation(panel.GetRotation() + 1.0f);
	});

	m_OnMouseClick = Subscribe<Smasher::Events::MouseButtonEvent>([this](Smasher::Events::MouseButtonEvent& event) {
		m_PanelPosition = sf::Vector2f(event.Position);
	});
}
