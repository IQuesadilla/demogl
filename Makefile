CFLAGS = -Wall -std=c++17 -O3
LIBS = -ldl -lpthread -lSDL2 -limgui -lfreetype -lpng -lbz2 -lz

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
		LIBS += -lGL -lbrotlidec-static -lbrotlicommon-static
		CXX = g++
    endif
    ifeq ($(UNAME_S),Darwin)
        VCPKGPLAT = osx
		LIBS += -mmacosx-version-min=13.6 -lbrotlidec -lbrotlicommon -framework CoreAudio -framework GameController -framework IOKit -framework AudioToolbox -framework CoreHaptics -framework CoreVideo -framework ForceFeedback -framework Metal -framework Foundation -framework Cocoa -framework CoreFoundation -framework OpenGL -liconv -framework CoreGraphics -framework Carbon
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
INCLUDE += -I./libQ/include/
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

bin/cvdemo: cvdemo.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS) -lopencv_videoio4 -lopencv_highgui4 -lopencv_core4 -lopencv_imgcodecs4 -lopencv_imgproc4 -lsharpyuv -ljpeg -ltiff -lwebp -llzma -framework OpenCL -framework AVFoundation -framework CoreMedia

bin/split: split.cpp scene.o scene_collada.o model.o model_cube.o model_window.o renderable.o shader.o camera.o xml.o origin.o model_blank.o
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS) -lopencv_videoio4 -lopencv_highgui4 -lopencv_core4 -lopencv_imgcodecs4 -lopencv_imgproc4 -lsharpyuv -ljpeg -ltiff -lwebp -llzma -framework OpenCL -framework AVFoundation -framework CoreMedia

scene.o: scene/scene.cpp scene/scene.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

scene_collada.o: scene/collada.cpp scene/collada.h scene/scene.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

renderable.o: renderable/renderable.cpp renderable/renderable.h model/model.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

model.o: model/model.cpp model/model.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

model_cube.o: model/cube.cpp model/cube.h model/model.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

model_window.o: model/window.cpp model/window.h model/model.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

model_blank.o: model/blank.cpp model/blank.h model/model.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

shader.o: libQ/src/shader.cpp libQ/include/shader.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

camera.o: libQ/src/camera.cpp libQ/include/camera.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

xml.o: libQ/src/xml.cpp libQ/include/xml.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

origin.o: origin/origin.cpp origin/origin.h
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	- rm bin/*
	- rm *.o
