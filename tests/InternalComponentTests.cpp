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

	SerializedComponent(int data1, const char* data3) :
		m_Data1(data1), m_Data3(data3), m_Data2(m_Data3.size()),
		Smasher::IComponent(), Smasher::ISerializeable() {}

	virtual void Serialize(Smasher::OutputArchive& out) const {
		out.WriteBytes(m_Data1);
		out.WriteBytes(m_Data2);
		out.WriteBytes(m_Data3.data(), m_Data2);
	}

	virtual void Deserialize(Smasher::InputArchive& in) {
		in.ReadBytes(m_Data1);
		in.ReadBytes(m_Data2);
		m_Data3.resize(m_Data2);
		in.ReadBytes(m_Data3.data(), m_Data2);
	}

	int GetData1() const { return m_Data1; }
	int GetData2() const { return m_Data2; }
	const std::string& GetData3() const { return m_Data3; }

private:
	std::string m_Data3;
	int m_Data1 = 0;
	std::size_t m_Data2 = 0;
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
	const std::string TEST_STR{ "300 Data" };

	SerializedComponent& component = pEntity->AddComponent<SerializedComponent>(100, "300 Data");
	
	EXPECT_EQ(100, component.GetData1());
	EXPECT_EQ(TEST_STR.size(), component.GetData2());
	EXPECT_EQ(TEST_STR, component.GetData3());

	Smasher::OutputArchive outputArchive(stream);
	Smasher::InputArchive inputArchive(stream);

	ASSERT_NO_THROW({ component.Serialize(outputArchive); });
	char buffer[64] = { 0 };
	// Get number of bytes left in stream
	std::streampos begin = stream.tellg(); // Save the stream start position
	int length = (int)stream.seekg(0, std::ios::end).tellg();
	length = std::min(length, (int)sizeof(buffer));

	stream.seekg(begin);
	stream.read(buffer, length);
	stream.seekg(begin);

	int data1 = 0;
	std::size_t data2 = 0;
	memcpy(&data1, &buffer[0], sizeof(int));
	memcpy(&data2, &buffer[sizeof(int)], sizeof(std::size_t));
	std::string data3 = &buffer[sizeof(int) + sizeof(std::size_t)];

	EXPECT_EQ(100, data1);
	EXPECT_EQ(TEST_STR.size(), data2);
	EXPECT_EQ(TEST_STR, data3);

	Smasher::Entity& entity2 = pState->AddEntity<Smasher::Entity>();
	SerializedComponent& newComponent = entity2.AddComponent<SerializedComponent>(5, "20 Data");

	EXPECT_EQ(5, newComponent.GetData1());
	EXPECT_EQ(std::string("20 Data").size(), newComponent.GetData2());
	EXPECT_EQ(std::string("20 Data"), newComponent.GetData3());

	ASSERT_NO_THROW({ newComponent.Deserialize(inputArchive); });

	EXPECT_EQ(100, newComponent.GetData1());
	EXPECT_EQ(TEST_STR.size(), newComponent.GetData2());
	EXPECT_EQ(TEST_STR, newComponent.GetData3());
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