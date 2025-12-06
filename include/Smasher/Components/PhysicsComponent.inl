namespace Smasher {

	template<class T>
	PhysicsComponent& PhysicsComponent::SetOnCollisionCallback(void (T::* method)(PhysicsCollision), T* instance) {
		return SetOnCollisionCallback(std::bind(method, instance, std::placeholders::_1));
	};
}