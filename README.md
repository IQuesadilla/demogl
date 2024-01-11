# demogl

## Load submodules
```bash
git submodule init
git submodule update
```

## Install libraries
```bash
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install sdl2
./vcpkg/vcpkg install glm
./vcpkg/vcpkg install "imgui[core,opengl3-binding,sdl2-binding,freetype]"
```

## Every way something has failed
- vcpkg may fail if pkg-config is not installed
- sdl2 install once failed with libsystemd error because of missing package python3-jinja2
