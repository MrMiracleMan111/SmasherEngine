#include "MidLayer.h"
#include "Components/TextComponent.h"
#include "Manifest.h"

MidLayer::~MidLayer() {

}

void MidLayer::Init()
{
	Smasher::Entity& entity = AddEntity();

	entity.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("Mid Layer")
		.SetFillColor(sf::Color::Magenta)
		.SetPosition(130.0f, 200.0f)
		.SetFontAsset<Smasher::Resources::Fonts::arial>();
}
