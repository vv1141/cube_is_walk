# Cube Is Walk - ECS demo

![](https://github.com/vv1141/cube_is_walk/blob/master/screenshots/demo.gif)

Cube Is Walk is a tiny puzzle game demonstrating the general principles of an Entity Component System (ECS) using [Flecs](https://www.flecs.dev/flecs/). All ECS functionality and game logic is contained in the [World](https://github.com/vv1141/cube_is_walk/blob/master/cpp/World.h) class, and the rest is mostly rendering and input handling implemented previously in [3D Engine Demo](https://github.com/vv1141/3d_engine_demo).

## Demonstrated ECS features

* Composing entities and utilising entity relationships
* Finding and iterating entities with queries
* Structuring scenes and update loops to pipelines and systems

## Game features

* Many different entity interactions
* 3D graphics using a custom rendering engine
* 5 puzzling levels

## Pre-built binaries

Pre-built binaries (x86 only) are available for Linux and Windows in [releases](https://github.com/vv1141/cube_is_walk/releases).

## Building from source

### Dependencies

* [OpenGL](https://www.opengl.org/) - rendering
* [GLEW](https://glew.sourceforge.net/) - OpenGL extension loading
* [GLM](https://github.com/g-truc/glm) - vector/matrix implementations and common operations
* [SFML](https://www.sfml-dev.org/) - window management, OpenGL context creation, input and font handling

The required packages on Debian, for example:

```bash
libgl1-mesa-dev
libglu1-mesa-dev
libglew-dev
libglm-dev
libsfml-dev
```

### Build and run:

```bash
git clone https://github.com/vv1141/cube_is_walk.git
cd cube_is_walk
cmake .
make
cd bin
./ecs_demo
```

## Screenshots

![](https://github.com/vv1141/cube_is_walk/blob/master/screenshots/00.png)
![](https://github.com/vv1141/cube_is_walk/blob/master/screenshots/01.png)
![](https://github.com/vv1141/cube_is_walk/blob/master/screenshots/02.png)
![](https://github.com/vv1141/cube_is_walk/blob/master/screenshots/03.png)
