#include <gtest/gtest.h>
#include "Core.h"
#include "BaseComponentManager.h"
#include "Components/CameraComponent.h"

class DummyGameState : public Smasher::GameState {
public:
	DummyGameState(Smasher::Engine& engine) : Smasher::GameState(engine) {}
};

class SerializedComponent : public Smasher::IComponent, Smasher::ISerializeable {
public:

	SerializedComponent(int data1, int data2, const char* data3) :
		m_Data1(data1), m_Data2(data2), m_Data3(data3),
		Smasher::IComponent(), Smasher::ISerializeable() {}

	virtual void Serialize(Smasher::OutputArchive& out) const {
		out.WriteBytes(m_Data1);
		out.WriteBytes(m_Data2);
		out.WriteBytes(m_Data3.c_str(), (std::size_t)(m_Data3.size() + 1));
	}

	virtual void Deserialize(Smasher::InputArchive& in) {
		in.ReadBytes(m_Data1);
		in.ReadBytes(m_Data2);
		in.ReadBytes(m_Data3);
	}

	int GetData1() const { return m_Data1; }
	int GetData2() const { return m_Data2; }
	const std::string& GetData3() const { return m_Data3; }

private:
	int m_Data1 = 0;
	int m_Data2 = 0;
	std::string m_Data3;
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

class SerializeComponentFixture : public EngineSetupFixture {};
class Transform2DComponentFixture : public EngineSetupFixture {};
class CameraComponentFixture : public EngineSetupFixture {};

// Serialize Component Tests
TEST_F(SerializeComponentFixture, SerializeComponent) {
	std::stringstream stream;

	SerializedComponent& component = pEntity->AddComponent<SerializedComponent>(100, 200, "300 Data");
	
	EXPECT_EQ(100, component.GetData1());
	EXPECT_EQ(200, component.GetData2());
	EXPECT_EQ(std::string("300 Data"), component.GetData3());

	Smasher::OutputArchive outputArchive(stream);
	Smasher::InputArchive inputArchive(stream);

	EXPECT_NO_THROW({ component.Serialize(outputArchive); });
	char buffer[17] = { 0 };
	stream.read(buffer, 17);

	int data1, data2;
	char* data3;
	memcpy(&data1, &buffer[0], sizeof(int));
	memcpy(&data2, &buffer[sizeof(int)], sizeof(int));
	data3 = &buffer[2 * sizeof(int)];

	EXPECT_EQ(100, data1);
	EXPECT_EQ(200, data2);
	EXPECT_STREQ("300 Data", data3);
}

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