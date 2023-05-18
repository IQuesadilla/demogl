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
./vcpkg/vcpkg install "imgui[core,opengl3-binding,sdl2-binding]"
```
