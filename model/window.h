
#include "glad/glad.h"
#include "SDL.h"
#include "shader.h"
#include "camera.h"
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <list>
#include <chrono>
#include <thread>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "model/model.h"


class Window : public Model
{
public:
  Window();
  ~Window();

  void update(bool DoDebugDraw);

private:
  cv::VideoCapture cap;
 
  void ThreadFunc();
  std::thread CameraThread;
  bool ShouldRunThread, NewFrame;

  std::mutex FrameMutex;
  cv::Mat CurrentFrame;
  float AspectRatio;

  struct {
    float exposure;
    float contrast;
  } CamValues;
};
