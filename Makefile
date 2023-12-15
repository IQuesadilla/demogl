CFLAGS = -Wall -std=c++17 -O3
LIBS = -ldl -lpthread -lSDL2 -limgui -lfreetype -lpng -lbrotlidec-static -lbrotlicommon-static -lbz2 -lz

ifeq ($(OS),Windows_NT)
#    CCFLAGS += -D WIN32
#    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
#        CCFLAGS += -D AMD64
#    else
#        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
#            CCFLAGS += -D AMD64
#        endif
#        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
#            CCFLAGS += -D IA32
#        endif
#    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        VCPKGPLAT = linux
		LIBS += -lGL
		CXX = g++
    endif
    ifeq ($(UNAME_S),Darwin)
        VCPKGPLAT = osx
		LIBS += -framework CoreAudio -framework GameController -framework IOKit -framework AudioToolbox -framework CoreHaptics -framework CoreVideo -framework ForceFeedback -framework Metal -framework Foundation -framework Cocoa -framework CoreFoundation -framework OpenGL -liconv -framework CoreGraphics -framework Carbon
		CXX = g++
    endif
    UNAME_P := $(shell uname -m)
    ifeq ($(UNAME_P),x86_64)
        VCPKGARCH = x64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        VCPKGARCH = x86
    endif
#    ifneq ($(filter arm%,$(UNAME_P)),)
#        CCFLAGS += -D ARM
#    endif
	VCPKGDIR = ./vcpkg/installed/$(VCPKGARCH)-$(VCPKGPLAT)/
endif

INCLUDE = -I.
INCLUDE += -I$(VCPKGDIR)/include/
INCLUDE += -I$(VCPKGDIR)/include/SDL2/
INCLUDE += -I$(VCPKGDIR)/include/glm/
LIBS += -L$(VCPKGDIR)/lib/


all: bin/sdlgl bin/sdlglimgui bin/sdlglimguitextured bin/curve bin/split

bin/sdlglimguitextured: sdlglimguitextured.cpp shader.o camera.o origin.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

bin/sdlglimgui: sdlglimgui.cpp shader.o camera.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

bin/sdlgl: sdlgl.cpp shader.o camera.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS) 

bin/curve: curve.cpp shader.o camera.o origin.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

bin/split: split.cpp scene.o model.o model_cube.o renderable.o shader.o camera.o origin.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

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
