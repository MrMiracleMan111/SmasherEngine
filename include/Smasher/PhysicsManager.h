#include "box2d/box2d.h"
#include "Smasher/Base.h"


namespace Smasher {
	// Manages the Box2D "global" state (Box2D world object and Box2D allocator)
	class SMASHER_API PhysicsManager {
	public:
		// non-copyable, moveable
		~PhysicsManager();
		PhysicsManager() = default;
		PhysicsManager(PhysicsManager&) = delete;
		PhysicsManager(PhysicsManager &&other) noexcept;
		PhysicsManager& operator=(PhysicsManager&) = delete;
		PhysicsManager& operator=(PhysicsManager &&other) noexcept;

		void Initialize(const b2WorldDef &worldDef);
		void Initialize(); // Initialize with Box2D defaults 

		void SetTimeStep(float timeStep) { m_TimeStep = timeStep; }
		void SetSubStepCount(int subStepCount) { m_SubStepCount = subStepCount; }
		void Step(Smasher::Millisecond delta); // Physics World Step

		const bool IsInitialized() const { return m_Initialized; };
		const b2WorldId& GetWorldId() const { return m_WorldId; };

		const float GetTimeStep() const { return m_TimeStep; }
		const int GetSubStepCount() const { return m_SubStepCount; }
	private:
		float m_TimeStep = Smasher::EngineConfig::BOX2D_TIMESTEP;
		int m_SubStepCount = Smasher::EngineConfig::BOX2D_SUBSTEP_COUNT;
		float m_Accumulator = 0.f;
		bool m_Initialized = false; // Has the physics world been generated
		b2WorldId m_WorldId = { 0 };
	};
}