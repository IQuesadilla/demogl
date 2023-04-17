CXX = g++
CFLAGS = -Wall
TARGET = demo
SDLINC = -I./vcpkg/installed/x64-linux/include/SDL2/
SDLLIB = $(wildcard ./vcpkg/installed/x64-linux/lib/*.a) -ldl -lpthread

all: demo

$(TARGET): main.cpp demogl.o
	$(CXX) $(CFLAGS) $(SDLINC) -o $@ $^ $(SDLLIB)

demogl.o: demogl.cpp demogl.h
	$(CXX) $(CFLAGS) $(SDLINC) -o $@ -c $<

clean:
	- rm $(TARGET)
	- rm wildcard *.o