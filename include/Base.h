#pragma once
#include <cstdint>
#include <chrono>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "plf_colony.h"
#include "Exceptions.h"
#include "Smasher_export.h"
// EngineAPI.hpp
#pragma once
#define SMASHER_API SMASHERENGINE_EXPORT

#define SMASHER_ADD_CAPABILITIES(args) \
virtual static constexpr uint32_t GetStaticCapabilities() { \
	return (args); \
}

// using fnv1a_hash 
constexpr uint64_t hash_str(const char* str) {
	uint64_t hash = 14695981039346656037ULL; // FNV offset basis
	while (*str) {
		hash ^= static_cast<uint64_t>(*str++);
		hash *= 1099511628211ULL; // FNV prime
	}
	return hash;
}

namespace Smasher {
	class GameState;
	class IComponent;
	class Entity;
	using Millisecond = std::chrono::milliseconds;
	using ResourceID = uint64_t;
	using ResourcePath = std::filesystem::path;
	using Degrees = float; // In Degrees

	template <typename T, typename U>
	concept ComponentManagerHasAddComponent = requires(T t, Entity & rEntity) {
		{ t.AddComponent(rEntity) } -> std::same_as<U&>;
	};

	template<class T>
	concept HasPathVariable = std::same_as<std::decay_t<decltype(T::PATH)>, ResourcePath>;

	template <class T>
	concept HasPathsVariable = std::same_as<std::decay_t<decltype(T::PATHS)>, ResourcePath*>;
	
	template <typename T>
	concept HasPathOrPathsVariable = HasPathVariable<T> || HasPathsVariable<T>;

	template<typename T>
	concept IComponentType = std::is_base_of_v<IComponent, T>;

	template <typename T>
	concept ManifestItemHasResourceID = requires(T) {
		T::ID; // Checks if 'member_name' is a valid member access
	};

	template <typename T>
	concept ManifestItemHasResourcePath = requires(T) {
		T::PATH; // Checks if 'member_name' is a valid member access
	};

	template <typename T>
	concept HasStaticInstantiateManager = requires(Smasher::GameState & arg) {
		T::StaticInstantiateManager(arg);
	};

	template <typename T>
	concept HasStaticRenderComponent = requires(T & comp, sf::RenderWindow & arg) {
		{ T::StaticRenderComponent(comp, arg) } -> std::same_as<void>;
	};

	template <typename T>
	concept HasStaticUpdateComponent = requires(T & comp, Smasher::Millisecond arg) {
		{ T::StaticUpdateComponent(comp, arg) } -> std::same_as<void>;
	};


	template <typename T>
	concept HasRenderCapability = requires(sf::RenderWindow & arg) {
		T::Render(arg);
	};

	template <typename T>
	concept HasUpdateCapability = requires(Millisecond & arg) {
		T::Update(arg);
	};

	enum class ResourceType {
		TEXTURE,
		FONT,
		AUDIO,
		SHADER,
		FILE,
		INVALID
	};

	enum class ComponentStatus {
		VALID,   // Component is active within a manager (not removed)
		INVALID, // Ready to be removed or just created (not assigned to manager yet)
		REMOVED  // Component was removed 
	};

	struct Keyboard {
	public:
		enum class KeyboardEventType : int {
			KEY_PRESS,
			KEY_RELEASE,
			KEY_HOLD
		};
	};

	struct Mouse {
	public:
		enum class MouseEventType {
			BUTTON_PRESS,
			BUTTON_RELEASE,
			SCROLL,
			MOUSE_MOVE
		};
	};

	enum class EventType {
		DummyEvent,
		DummyEventExtra,
		KeyboardEvent,
		MouseButtonEvent,
		MouseScrollWheelEvent,
		MouseMoveEvent,
		END
	};
}