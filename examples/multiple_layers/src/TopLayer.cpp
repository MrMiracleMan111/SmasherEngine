#include "TopLayer.h"
#include "Components\TextComponent.h"
#include "Manifest.h"

TopLayer::~TopLayer()
{
}

void TopLayer::Init()
{
	Smasher::Entity& entity = AddEntity();

	entity.AddComponent<Smasher::TextComponent>()
		.UseDefaults()
		.SetString("Top Layer")
		.SetFillColor(sf::Color::Red)
		.SetPosition(160.0f, 200.0f)
		.SetFontAsset<Smasher::Resources::Fonts::arial>();
}
