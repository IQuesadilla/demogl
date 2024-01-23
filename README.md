# demogl

## Load submodules
```bash
git submodule init
git submodule update
```

## To Build
```bash
cmake -B build/ -S . 
cmake --build build/ --target [sdlgl,sdlglimgui,sdlglimguitextured,split]
./build/[sdlgl,sdlglimgui,sdlglimguitextured,split]
```

## Every way something has failed
- vcpkg may fail if pkg-config is not installed
- sdl2 install once failed with libsystemd error because of missing package python3-jinja2
