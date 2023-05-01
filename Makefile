CXX = g++
CFLAGS = -Wall -std=c++17 -O3
INCLUDE = $(SDLINC) $(GLMINC) -I. 

IMGUIINC = -I./vcpkg/installed/x64-linux/include/
SDLINC = -I./vcpkg/installed/x64-linux/include/SDL2/
GLMINC = -I./vcpkg/installed/x64-linux/include/glm/
LIBS = $(wildcard ./vcpkg/installed/x64-linux/lib/*.a) -ldl -lpthread

all: bin/sdlgl bin/sdlglimgui bin/sdlglimguitextured bin/curve bin/split

bin/sdlglimguitextured: sdlglimguitextured.cpp shader.o camera.o origin.o
	$(CXX) $(CFLAGS) $(INCLUDE) $(IMGUIINC) -o $@ $^ $(LIBS) -lGL

bin/sdlglimgui: sdlglimgui.cpp shader.o camera.o
	$(CXX) $(CFLAGS) $(INCLUDE) $(IMGUIINC) -o $@ $^ $(LIBS) -lGL

bin/sdlgl: sdlgl.cpp shader.o camera.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS) -lGL

bin/curve: curve.cpp shader.o camera.o origin.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS) -lGL

bin/split: split.cpp scene.o model.o renderable.o shader.o camera.o
	$(CXX) $(CFLAGS) $(INCLUDE) $(IMGUIINC) -o $@ $^ $(LIBS) -lGL

scene.o: scene/scene.cpp scene/scene.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

renderable.o: renderable/renderable.cpp renderable/renderable.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

model.o: model/model.cpp model/model.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

model_cube.o: model/cube.cpp model/cube.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

shader.o: shader/shader.cpp shader/shader.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

camera.o: camera/camera.cpp camera/camera.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

origin.o: origin/origin.cpp origin/origin.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	- rm bin/*
	- rm *.o