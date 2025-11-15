namespace Smasher {

	template<class T>
	AABBPhysicsComponent& AABBPhysicsComponent::SetOnCollisionCallback(void (T::* method)(AABPPhysicsCollision), T* instance) {
		return SetOnCollisionCallback(std::bind(method, instance, std::placeholders::_1));
	};
}