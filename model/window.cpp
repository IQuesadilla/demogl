#include "window.h"
#include "imgui.h"

#define I 1.0f

static const GLfloat WindowVerts[] = {
    // South
    -I, I, 0.0f,
    -I,-I, 0.0f,
     I, I, 0.0f,
     I, I, 0.0f,
    -I,-I, 0.0f,
     I,-I, 0.0f,
};

static const GLfloat WindowUV[] = {
    // Top
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
};

Window::Window() : Model()
{
  std::vector<GLfloat> vertDataVec;
  vertDataVec.resize(sizeof(WindowVerts) / sizeof(GLfloat));
  memcpy(vertDataVec.data(),WindowVerts,sizeof(WindowVerts)); 

  std::vector<GLfloat> uvDataVec;
  uvDataVec.resize(sizeof(WindowUV) / sizeof(GLfloat));
  memcpy(uvDataVec.data(),WindowUV,sizeof(WindowUV));

  CollisionVerts = vertDataVec;

  doGenerateMipmap = false;
  setModel(vertDataVec);
  setTex(cv::Mat(), uvDataVec);

  AspectRatio = 1.f;

  int deviceID = -1;
  int apiID = cv::CAP_ANY;

  cap.open(deviceID, apiID);
  //cap.set(cv::CAP_PROP_ORIENTATION_META, 0.0);
  cap.set(cv::CAP_PROP_CONVERT_RGB,1.0);

  if (!cap.isOpened())
  {
    std::cerr << "ERROR! Unable to open camera" << std::endl;
    return;
  }

  isEnclosed = false;
  TCount = 2;
  Info.Title = "window";
  GLInit();

  ShouldRunThread = true;
  NewFrame = false;
  CameraThread = std::thread(&Window::ThreadFunc, this);
}

Window::~Window() {
  ShouldRunThread = false;
  CameraThread.join();
}

void Window::update(bool DoDebugDraw)
{
  if (DoDebugDraw)
  {
    //ImGui::SeparatorText("Camera Manager");
    //if (ImGui::SliderFloat("Contrast", &CamValues.contrast, 0.0f, 255.0f)) cap.set(cv::CAP_PROP_CONTRAST,CamValues.contrast);
    //if (ImGui::SliderFloat("Exposure", &CamValues.exposure, 0.0f, 255.0f)) cap.set(cv::CAP_PROP_EXPOSURE,CamValues.exposure);
  }

  FrameMutex.lock();
  if (NewFrame) {
    Info.ImpliedTransform[0][0] = AspectRatio;
    setTex(CurrentFrame);
    NewFrame = false;
  }
  FrameMutex.unlock();
}

void Window::ThreadFunc() {
  while (ShouldRunThread) {
    cv::Mat TempFrame;
    bool success = cap.isOpened() &&
                   cap.grab() &&
                   cap.retrieve(TempFrame) &&
                   !TempFrame.empty();
    if (!success) CurrentFrame = cv::Mat(20, 20, CV_8UC3, cv::Scalar(0, 0, 0));

    cv::cvtColor(TempFrame, TempFrame, cv::COLOR_BGR2RGB);
    if (!TempFrame.isContinuous()) {
      TempFrame = TempFrame.clone(); // Ensure the data is continuous
    }

    //cv::Mat UnlockedFrame = TempFrame.clone();
    FrameMutex.lock();
    AspectRatio = float(TempFrame.cols) / float(TempFrame.rows);
    CurrentFrame = TempFrame;//UnlockedFrame;
    NewFrame = true;
    FrameMutex.unlock();
  }
}
/*
Window::Window(Window *_new)
{
  //std::swap(this,_new);
  this = _new;
}*/
