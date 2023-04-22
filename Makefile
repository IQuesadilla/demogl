CXX = g++
CFLAGS = -Wall -std=c++17 -O3
INCLUDE = $(SDLINC) $(GLMINC)

SDLINC = -I./vcpkg/installed/x64-linux/include/SDL2/
GLMINC = -I./vcpkg/installed/x64-linux/include/glm/
LIBS = $(wildcard ./vcpkg/installed/x64-linux/lib/*.a) -ldl -lpthread

all: bin/gl

bin/gl: gl.cpp shader.o camera.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS) -lGL

shader.o: shader/shader.cpp shader/shader.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

camera.o: camera/camera.cpp camera/camera.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	- rm bin/*
	- rm *.o