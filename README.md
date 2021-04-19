T1 - Graphics Engine
======
This is a Graphics Renderer built with C++, OpenGL, CMake.  

Feature List 
------
* Fully functioning (custom) math library (vectors, matrices, transformations, utility functions):
* Engine utility namespace:
* Configure and enable debugging and logging utility.
* Shader object w/ custom pre-processor (defines, includes):
* Material pipeline; configure render state in materials, together with shader, uniform configuration and samplers used.
* Define Mesh object and encompassing Model class.
    1. Assimp
* Flexible Framebuffer configuration utility:
* Create Render Buffer, draw commands encapsulated in push to render buffer w/ relevant state:
* Configure Deferred rendering pipeline:
* Functioning PBR render pipeline.
* Post-processing
* Optimization:
* GUI 
    1. IMGUI		

Building
------
T1 is developed on **Windows** with Visual Studio and also builds on **Linux**. macOS is not
supported (it stops at OpenGL 4.1).

Requirements:

* CMake 3.15 or newer (3.x)
* Git, and an internet connection the first time CMake runs
* a C++14 compiler: Visual Studio 2017 or 2019 on Windows, GCC 5 or newer on Linux
* a 64-bit system with a GPU and driver that support OpenGL 4.3 core profile

Third-party libraries:

| Library    | Version     | Where it comes from                                         |
|------------|-------------|-------------------------------------------------------------|
| GLFW       | 3.2.1       | downloaded and built by CMake (`FetchContent`)              |
| Assimp     | 3.3.1       | downloaded and built by CMake (`FetchContent`)              |
| glad       | GL 4.5 core | included in `t1_engine/glad/`                               |
| Dear ImGui | 1.52        | included in `t1_engine/imgui/`                              |
| stb_image  | 2.12        | included in `t1_engine/stb/`                                |

The first time CMake configures the project it clones GLFW and Assimp at those exact versions into
`build/_deps/` and builds them as static libraries together with T1, so no prebuilt binaries are
needed and nothing has to be copied next to the demos. Assimp is the largest part of the first
build; later builds only recompile what changed.

CMake is run from inside the `build/` folder. That folder also holds the `shaders/`, `textures/`
and `meshes/` the demos load, so the demos must be started with `build/` as the working directory.

### Windows (Visual Studio)

Install Visual Studio 2017 or 2019 with the *Desktop development with C++* workload, then generate
the Visual Studio solution and open it:

    cd build
    cmake .. -A x64
    start T1.sln

CMake picks the newest Visual Studio it finds. To choose one explicitly, add
`-G "Visual Studio 16 2019"` or `-G "Visual Studio 15 2017"`.

Build the `Release` configuration and press F5. `demo` is the startup project and runs with `build/`
as its working directory.

### Linux

Builds on Ubuntu 20.04 (GCC 9, CMake 3.16). GLFW needs the X11 development headers:

    sudo apt install build-essential cmake git xorg-dev libgl1-mesa-dev zlib1g-dev
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j4
    ./demo        # or ./foundry

Demos
------
Both demos are built by default and must run with `build/` as the working directory
(to switch demos, right-click the project in Visual Studio → *Set as Startup Project*).

* **`demo`** — the Sponza atrium with torches, a plasma orb and baked irradiance probes.
* **`foundry`** — *Foundry Yard*: an industrial yard built from the engine's procedural meshes and
  bundled textures. A gallery of PBR materials (gold, copper, plastic, rusted metal, glass,
  checker) on lantern-lit pedestals, a spinning gold/copper/plastic gyroscope around a plasma orb,
  walls of stacked crates, orbiting firefly lights, a day/dusk/night sun with shadows, and an
  optional 128-light show for the deferred renderer.

  Start-up flags: `--cinematic` (orbiting presentation camera), `--lightshow`, `--day`, `--night`.

Controls (both demos): `WASD`/arrows move, `Q`/`E` down/up, mouse to look, `Tab` toggles the GUI.
Foundry Yard also has: `C` cinematic camera, `N` cycle day/dusk/night, `L` light show,
`1`/`2` custom post-processing (invert / blue tint), plus a *Foundry Yard* GUI panel.

Credits
------
T1 is derived from [Cell](https://github.com/JoeyDeVries/Cell) by Joey de Vries, licensed under
[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/). It has been modified and extended.
