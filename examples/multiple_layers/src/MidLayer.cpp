#include "MidLayer.h"
#include "Smasher/Components/TextComponent.h"
#include "Manifest.h"
#include "Smasher/UI.h"
#include "Smasher/Interpolation.h"

MidLayer::~MidLayer() {

}

void MidLayer::Update(Smasher::Millisecond delta) {
	if (m_UIPanelPtr != nullptr) {
		m_UIPanelPtr->SetPosition(m_PanelPosition.Get());
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
	//GetEngine().GetResourceManager().SetResourceDirectory(Smasher::Manifest::Metadata::RESOURCES_DIRECTORY);
	//std::shared_ptr<Smasher::ShaderResource> pShader = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Manifest::Shaders::basic_ui_pShader, Smasher::ShaderResource>();
	//sf::Glsl::Mat4 viewProjectionMatrix = sf::Glsl::Mat4(GetEngine().GetWindow().getView().getTransform().getMatrix());
	//pShader->GetShader().setUniform("ViewProjectionMatrix", viewProjectionMatrix);

	//Smasher::UIPanelComponentManager &rCompManager = static_cast<Smasher::UIPanelComponentManager&>(GetComponentManager<Smasher::UIPanelComponent>());
	//rCompManager.SetShaderResource(pShader);

	m_PanelPosition.Set(sf::Vector2f(0.f, 0.f));
	m_PanelPosition.SetCurve(Smasher::InterpolationType::EASE_OUT);
	m_PanelPosition.SetDuration(Smasher::Millisecond{ 1000 });

	Smasher::Entity &entity = AddEntity();
	Smasher::Entity &fpsTracker = AddEntity();

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

	Smasher::Entity &uiPanel = AddEntity();

	std::shared_ptr<Smasher::TextureResource> pTexture = GetEngine().GetResourceManager().GetOrLoadResource<Smasher::Manifest::Textures::small_art, Smasher::TextureResource>();

	Smasher::UIPanelComponent &panel = uiPanel.AddComponent<Smasher::UIPanelComponent>()
		.SetBackgroundColor(sf::Color::Yellow)
		.SetBackgroundColor(Smasher::UIPanelCorner::LEFT, sf::Color::Blue)
		.SetBorderRadius(Smasher::UIPanelCorner::TOP_LEFT, 60.f)
		.SetBorderColor(sf::Color::Red)
		.SetBorderColor(Smasher::UIPanelCorner::TOP_LEFT, sf::Color::Green)
		.SetBorderColor(Smasher::UIPanelCorner::TOP_RIGHT, sf::Color::Blue)
		.SetBorderColor(Smasher::UIPanelCorner::BOTTOM_RIGHT, sf::Color::Magenta)
		.SetTexture(pTexture)
		.SetBorderThickness(10.0f)
		.SetPosition(100.0f, 60.0f)
		.SetScale(100.0f, 200.0f)
		//.SetClipRect(sf::IntRect{0, 0, 100, 100})
		//.SetClipRotation(Smasher::Degrees{ 10.0f })
		.SetDepth(0.4f);
	m_UIPanelPtr = &panel;
	panel.SetOnHoverCallback([&panel](Smasher::Events::MouseMoveEvent &event) {
		if ((bool)(panel.GetPanelState() & Smasher::UIPanelState::HOVERED)) {
			sf::Color tmp = sf::Color::Cyan;
			tmp.a = 40;
			panel.SetBackgroundColor(tmp);
		}
		else {
			sf::Color tmp = sf::Color::Magenta;
			tmp.a = 40;
			panel.SetBackgroundColor(tmp);
		}
	});

	m_OnMouseMove = Subscribe<Smasher::Events::MouseMoveEvent>([&panel](Smasher::Events::MouseMoveEvent &event) {
		//panel.SetPosition(sf::Vector2f(event.Position));
		panel.SetClipRotation(panel.GetClipRotation() + 1.0f);
		panel.SetRotation(panel.GetRotation() + 1.0f);
	});

	m_OnMouseClick = Subscribe<Smasher::Events::MouseButtonEvent>([this](Smasher::Events::MouseButtonEvent &event) {
		m_PanelPosition = sf::Vector2f(event.Position);
	});
}
