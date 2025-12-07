#pragma once

#include "Smasher/Base.h"
#include "Smasher/IComponent.h"
#include "Smasher/ISerializeable.h"

class SavedComponent : public Smasher::IComponent, public Smasher::ISerializeable {

public:
	SavedComponent() : Smasher::ISerializeable() {}

	const Smasher::Vec2 &GetPosition() const;
	const Smasher::Vec2 &GetVelocity() const;
	const Smasher::Degrees GetRotation() const;
	void Serialize(Smasher::OutputArchive &out) override;
	void Deserialize(Smasher::InputArchive &in) override;


private:
	void SaveState();
	void UpdateBallComponent();

	Smasher::Vec2 m_SavedPos = {0, 0};
	Smasher::Vec2 m_SavedVelocity = {0, 0};
	Smasher::Degrees m_SavedRot = 0;
};