CXX = g++
CFLAGS = -Wall -std=c++17 -O3
INCLUDE = $(SDLINC) $(GLMINC)

SDLINC = -I./vcpkg/installed/x64-linux/include/SDL2/
GLMINC = -I./vcpkg/installed/x64-linux/include/glm/
LIBS = $(wildcard ./vcpkg/installed/x64-linux/lib/*.a) -ldl -lpthread

all: bin/demo bin/gl

bin/demo: main.cpp demogl.o
	$(CXX) $(CFLAGS) $(SDLINC) -o $@ $^ $(LIBS)

bin/gl: gl.cpp shader/shader.o camera/camera.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS) -lGL

demogl.o: demogl.cpp demogl.h
	$(CXX) $(CFLAGS) $(SDLINC) -o $@ -c $<

shader/shader.o: shader/shader.cpp shader/shader.h
	$(CXX) $(CFLAGS) $(GLMINC) -o $@ -c $<

camera/camera.o: camera/camera.cpp camera/camera.h
	$(CXX) $(CFLAGS) $(GLMINC) -o $@ -c $<

clean:
	- rm bin/*
	- rm *.o