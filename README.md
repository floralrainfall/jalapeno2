# Jalapeno 2
Jalapeno2 is a 3D engine licensed under the Mozilla Public License version 2.0, a copy of license is in `/LICENSE`. The engine uses BGFX, ImGui, Assimp, OpenAL and optionally you can use `easy_profiler` although its support is limited.

In this repository you will find:

1. `engine`, the directory that contains the Jalapeno2 engine source code.
2. `jp2empireofages`, a real-time-strategy game
3. `sweetcombatguys`, a first person shooter game. This game requires `pak0.pk3` from Quake 3 to be extracted in `data/scg/quake` in order for backward-compatibility with Quake 3 map textures, and for SCG maps that use Quake 3 assets.
4. `test`, a testing executable for the Jalapeno2 engine
5. `totalevo`, an evolution simulator
6. `xtralibs/jpbsp`, a Quake 3 BSP loading library for Jalapeno2

## Compiling

1. Clone the repository: `git clone https://github.com/floralrainfall/jalapeno2`
2. Make sure to update the submodules: `git submodule update --init`
3. Run cmake in a build dir, e.g: `mkdir build &&  cd build && cmake ..`
4. Build (`make` or `ninja` or `msbuild`, depends on what you used with cmake)
5. Run the compile shaders script, for example on Linux you would run `./compileShaders-linux.sh`
6. You can now run the game!

### Simple script to automate this (Linux)
    git clone https://github.com/floralrainfall/jalapeno2
    git submodule update --init
    mkdir build
    cd build
    cmake -G Ninja ..
    ninja
    cd ..
    chmod +x compileShaders-linux.sh
    ./compileShaders-linux.sh
Please fix this if it sucks