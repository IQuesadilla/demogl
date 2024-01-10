#define GL_GLEXT_PROTOTYPES 1

#include "SDL.h"
#include <SDL_opengl.h>
#include "shader.h"
#include "camera.h"
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <list>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "model/model.h"


class Window : public Model
{
public:
    Window();
    Window(Window *_new);

    void update(bool DoDebugDraw);

private:
    cv::VideoCapture cap;
    cv::Mat frame;
    
    struct {
        float exposure;
        float contrast;
    } CamValues;
};
