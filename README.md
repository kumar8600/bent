# bent

An [Entity-component-system] library

* modern C++
* type-safe
* fast
  * benchmarked
  * copy gc makes it possible
* tiny
  * wrap this library for practical use
* [MIT license](LICENSE)

## prerequisites

* CMake
  * \>= 3.1
* modern C++ compiler
  * Visual Studio
    * \>= 2015
  * GCC
    * \>= 5.0
  * Clang
    * not tested

## install

wait

## tutorial

before everything, include `bent/bent.hpp`.

```cpp
#include <bent/bent.hpp>
```

### world

`bent::World` manages all entities and components attached to them.

```cpp
bent::World world;
```

### entities

`bent::World::Create` creates an entity and returns `bent::EntityHandle`.

```cpp
auto entity = world.Create();
```

to destroy entity:

```cpp
entity.Destroy();
```

### components

just a struct/class that copyable/movable. pod class is recommended.

```cpp
struct Position
{
	Position(float x, float y) : x(x), y(y) {}
	float x, y;
};

struct Velocity
{
	Velocity(float x, float y) : x(x), y(y) {}
	float x, y;
};

struct Renderable // empty class is allowed
{
};
```

to add component for entities:

```cpp
entity.Add<Position>(1.0f, 2.0f);
entity.Add<Velocity>(0.5f, 0.0f);
entity.Add<Renderable>();
```

to remove:

```cpp
entity.Remove<Velocity>();
```

to get:

```cpp
Position* pos = entity.Get<Position>();
```

this pointer is invalidated when `bent::World::CollectGarbage` is called.

### systems

systems implement behavior.

(There are no classes likes `bent::System`. If you want, create your own.)

`bent::World::entities_with` returns entities have components queried.

```cpp
// in main loop

// movement system
for (auto& entity : world.entities_with<Position, Velocity>())
{
	auto pos = entity.Get<Position>();
	auto vel = entity.Get<Velocity>();
	pos->x += vel->x;
	pos->y += vel->y;
}

// render system
for (auto& entity : world.entities_with<Renderable, Position>())
{
	auto pos = entity.Get<Position>();
	std::cout << entity.name() << ": " << pos->x << ", " << pos->y << std::endl;
}
```

### garbage collection

call below in begin or end of every frame:

```cpp
world.CollectGarbage();
```

every destroyed entities and removed components are sweeped and components are sorted for better cache coherency.
in other words, this is copy gc.

when this function is called, all pointer refering components are invalidated.

### full example

```cpp

#include <bent/bent.hpp>
#include "components/position.hpp"
#include "components/velocity.hpp"
#include "components/renderable.hpp"

int main()
{
	bent::World world;
	
	auto entity = world.Create("name");
	entity.Add<Position>(1.0f, 1.0f);
	entity.Add<Velocity>(1.5f, 1.5f);
	entity.Add<Renderable>();
	
	// main loop
	while (1)
	{
		world.CollectGarbage(); // sweeps removed entities and components. (And some optimizations)
		
		// movement system
		for (auto& entity : world.entities_with<Position, Velocity>())
		{
			auto pos = entity.Get<Position>();
			auto vel = entity.Get<Velocity>();
			pos->x += vel->x;
			pos->y += vel->y;
		}
		
		// render system
		for (auto& entity : world.entities_with<Renderable, Position>())
		{
			auto pos = entity.Get<Position>();
			std::cout << entity.name() << ": " << pos->x << ", " << pos->y << std::endl;
		}
	}
}

```

## optional features

### component access without type

to access components dynamically, use functions below.

```cpp
void bent::EntityHandle::AddFrom(const std::string& component_name, const void * value);
void bent::EntityHandle::AddFromMove(const std::string& component_name, void * value);
void bent::EntityHandle::Remove(const std::string& component_name);
void* bent::EntityHandle::Get(const std::string& component_name);

bent::View bent::World::entities_with(int argc, const char ** argv);
```

before call functions above, you must register components.

```cpp
template <typename T>
void bent::RegisterComponent(const std::string& name);
```

## Special thanks

this library is inspired by below awesome libraries

* [ayebear/es]
* [alecthomas/entityx]
* [Catchouli/coment]
* [miguelmartin75/anax]

and [T-machine.org].

[Entity-component-system]: http://en.wikipedia.org/wiki/Entity_component_system
[alecthomas/entityx]: https://github.com/alecthomas/entityx
[ayebear/es]: https://github.com/ayebear/es
[Catchouli/coment]: https://github.com/Catchouli/coment
[miguelmartin75/anax]: https://github.com/miguelmartin75/anax
[T-machine.org]: http://t-machine.org/