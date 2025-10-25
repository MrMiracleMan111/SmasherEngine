# SmasherEngine

## Build
### CMake Dependencies
- [SFML 2.6.x](https://github.com/SFML/SFML/tree/2.6.x)
- [GLEW](https://github.com/nigels-com/glew)
- OpenGL

For **Linux** users, you likely will **NOT** be able to just use:
```bash
sudo apt-get install libsfml-dev
```
since that will install a more recent version of SFML (SFML 3.0 as of writing this). Instead, you will have to compile and install [SFML 2.6.x](https://github.com/SFML/SFML/tree/2.6.x).

If you are on **Windows** you will most likely have to clone the [SFML 2.6.x](https://github.com/SFML/SFML/tree/2.6.x) and [GLEW](https://github.com/nigels-com/glew) repositories, then follow the instructions to install them. To my knowledge, **Windows** and **Linux** users shouldn't have to do anything to make **OpenGL** accessible to **CMake**. 

### IMPORTANT!!!!
When compiling dependecies, make sure your **CMake** generator is consistent. For example, if you use **Visual Studio 17 2022** generator for [SFML 2.6.x](https://github.com/SFML/SFML/tree/2.6.x), make sure you also use **Visual Studio 17 2022** generator for [GLEW](https://github.com/nigels-com/glew) and this project.

### Linux Compiling Example
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DGENERATE_DOCS=FALSE ..
make
```

### Windows Compiling Example
I recommend using **Visual Studio** (in my case *Visual Studio 17 2022*) as your generator you avoid headaches.
```bash
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Debug -DGENERATE_DOCS=FALSE ..
make
```

## Manifest Tool
The Resource Manager will use a "*Resource.h*" header file to store resource
paths and UUIDs. The `ManifestHeaderGenerator.py` script is used to generate these headers.

```bash
python ManifestHeaderGenerator.py --files Mainfest.json
```

`Manifest.json`
```json
{
    "Textures":  {
          "player_sprite": "assets/textures/player.png",
          "enemy_sprite": "assets/textures/enemy.png",
    },
    "Sounds": {
          "gunshot": "assets/audio/gunshot.wav",
          "bg_music_1": "assets/songs/bg1.wav",
    }
}
```

`Manifest.h`
```C++
#pragma once
#include"ResourceUtils.h"

namespace Resources {
	namespace Textures {
		namespace player_sprite {
			constexpr ResourceID player_sprite_ID = hash_str("assets/textures/player.png");
			ResourcePath player_sprite_PATH { "assets/textures/player.png" };
		}
		namespace enemy_sprite {
			constexpr ResourceID enemy_sprite_ID = hash_str("assets/textures/enemy.png");
			ResourcePath enemy_sprite_PATH { "assets/textures/enemy.png" };
		}
	}
	namespace Sounds {
		namespace gunshot {
			constexpr ResourceID gunshot_ID = hash_str("assets/audio/gunshot.wav");
			ResourcePath gunshot_PATH { "assets/audio/gunshot.wav" };
		}
		namespace bg_music_1 {
			constexpr ResourceID bg_music_1_ID = hash_str("assets/songs/bg1.wav");
			ResourcePath bg_music_1_PATH { "assets/songs/bg1.wav" };
		}
	}
}
```

## Smasher Engine

```c++
#include "Smasher/Core.h"
# include "Smasher/Components/TextComponent.h"

int main() {
	Smasher::Engine engine(640, 420);
	Smasher::Layer& layer = engine.AddLayer<Smasher::GameState>();
    layer.Activate();
	Smasher::Entity entity = layer.AddEntity<Smasher::Entity>();
	entity
    .AddComponent<TextComponent>()
        .SetString("Hello World!")
        .SetFontSize(30)
        .SetOutlineThickness(10);
    engine.Run();
}
```

### Engine


### Core
The Core header includes all necessary managers.

### Entity
Entities have no inherent logic. They are a **Composition** of Components.

Entities **CANNOT** have multiple components of the **SAME TYPE**

On the front end, components can be added and retrieved from entities like so:
`AddComponent<T>(args...)`
`GetComponent<T>()`

On the back end, the `Entity` will handle constructing the component and then pass ownership
to the corresponding `IComponentManager`.

## Component System

### Components
Components instances should be purely data. All component logic will be handled by
Component managers.

#### Generic Component Methods
If you want to use a custom Component Manager for your component you will need to define the
`std::unique_ptr<YourComponentManagerType> StaticInstantiateManager(GameState&)` method in your Component class.

For example the component below will utilize `GenericRenderableComponentManager<<MyComponent>` and
will define its own update method.
```C++
class MyComponent : public Component {
	static void StaticRenderComponent(sf::RenderWindow& window) {
		// ...
	}

	static void StaticUpdateComponent(Millisecond delta) {
		// ...
	}
}
```

In the example below a custom manager is used by the component `MyComponent`.
An override for `AddComponent(std::unique_ptr<Component> component)` since this Manager
will be using a linked list `std::list` instead of `std::vector`to store its components (`std::vector` is recommended).
```C++
class MyComponentManager : public ComponentManager {
public:
	MyComponentManager(GameState& state) : ComponentManager(state) {
		// ...
	}
    MyComponentManager(MyComponentManager&&) = default; // GameState

	void Update(Millisecond delta) {
		// Update all components
	}

	// [OPTIONAL METHOD]
	void AddComponent(std::unique_ptr<Component> component) override {
			component->MakeValid();
			m_Components.push_back(std::move(component));
	}

private:
	std::vector<std::unique_ptr<Component>> m_Components;
}

class MyComponent : public Component {
public:
    MyComponent(Smasher::Entity& entity) : Component(entity) {
        // ...
    }

    static MyComponentManager StaticInstantiateManager(GameState& state) {

    }
}
```

If you do not define the `StaticInstantiateManager(GameState&)` method, the Game engine will assume
you want to use the `GenericComponentManager<T>` or `GenericRenderableComponentManager<T>` to handle your component.

When using `GenericComponentManager<T>` or `GenericRenderableComponentManager<T>`
If you want your component to be updated or rendered will need to define the following methods in your Component class:
`StaticRenderComponent(sf::RenderWindow& window)`
`StaticUpdateComponent(Milliseconds delta)`


### ComponentManagers

Component Managers will be **Lazily Loaded** when `GameState.GetComponentManager<T>()` is called.
By default, the `GameState` will instantitate `GenericComponentManager<T>` .

However, Components may define the `static std::unique_ptr<CustomComponentManagerType> StaticInstantiateManager(GameState&)` method.
For shorthand they may instead use the macro `SMASHER_USE_COMPONENT_MANAGER(CustomComponentManagerType)`

All component managers inherit from the `IComponentManager` abstract class.
`Update(Millisecond delta)`

Users are expected to inherit the abstract `IComponent` class.

### Generic Component Manager
The Generic Component Manager is designed to remove the boilerplate code needed when creating
custom Component Managers for custom Components. If your Components require any optimizations
when updating, defining your own Component Manager is recommended over `GenericComponentManager<T>`.

If a custom component type needs to be updated instead of component logic being defined in a separate manager class, this manager expects
components to have a static method `StaticUpdateComponent(Millisecond delta)` to update component.

If the component doesn't need to be continuously updated (ex. a Transform is purerly data without any logic),
then forgoing defining `StaticUpdateComponent(Millisecond delta)` will prevent any updates.

The effect is offloading the update code to the component to remove the need for
new Component Managers for every component (especially useful for simple components with little logic)
	

This Component Manager will also check if the template T has a "StaticUpdateComponent" before trying
to run it. This removes need for "StaticUpdateComponent" implementation on purely
data components (Position, Rotation, etc.) where it wouldn't do anything.
