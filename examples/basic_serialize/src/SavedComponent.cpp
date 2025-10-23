#include "SavedComponent.h"
#include ""Smasher/ISerializeable.h""
#include "BallComponent.h"
#include "Smasher/Entity.h"
#include "Components/DrawableComponent.h"

void SavedComponent::Serialize(Smasher::OutputArchive& out) {
	SaveState();

	out.WriteBytes(m_SavedPos.data.x);
	out.WriteBytes(m_SavedPos.data.y);
	out.WriteBytes(m_SavedVelocity.data.x);
	out.WriteBytes(m_SavedVelocity.data.y);
	out.WriteBytes((float)m_SavedRot);
}

void SavedComponent::Deserialize(Smasher::InputArchive& in) {
	float rotation;

	in.ReadBytes(m_SavedPos.data.x);
	in.ReadBytes(m_SavedPos.data.y);
	in.ReadBytes(m_SavedVelocity.data.x);
	in.ReadBytes(m_SavedVelocity.data.y);
	in.ReadBytes(rotation);

	m_SavedRot = (Smasher::Degrees)(rotation);

	UpdateBallComponent();
}

void SavedComponent::SaveState()
{
	BallComponent& ballComp = GetEntity().GetComponent<BallComponent>();
	Smasher::DrawableComponent& drawableComp = GetEntity().GetComponent<Smasher::DrawableComponent>();

	sf::Vector2f pos = drawableComp.GetPosition();
	m_SavedPos = {drawableComp.GetPosition().x, drawableComp.GetPosition().y};
	m_SavedVelocity = {ballComp.GetVelocity().x, ballComp.GetVelocity().y};
	m_SavedRot = (float)drawableComp.GetRotation();
}

void SavedComponent::UpdateBallComponent()
{
	GetEntity().GetComponent<BallComponent>()
		.SetVelocity(sf::Vector2f(m_SavedVelocity.data.x, m_SavedVelocity.data.y));

	GetEntity().GetComponent<Smasher::DrawableComponent>()
		.SetPosition(sf::Vector2f(m_SavedPos.data.x, m_SavedPos.data.y))
		.SetRotation(Smasher::Degrees(m_SavedRot));
}
