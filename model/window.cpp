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

    int deviceID = 0;
    int apiID = cv::CAP_ANY;

    cap.open(deviceID, apiID);
    cap.set(cv::CAP_PROP_CONVERT_RGB,1.0);

    isEnclosed = false;
    TCount = 2;
    name = "window";

    if (!cap.isOpened())
    {
        std::cerr << "ERROR! Unable to open camera" << std::endl;
        return;
    }
} 

void Window::update(bool DoDebugDraw)
{
    if (DoDebugDraw)
    {
        //ImGui::SeparatorText("Camera Manager");
        if (ImGui::SliderFloat("Contrast", &CamValues.contrast, 0.0f, 255.0f)) cap.set(cv::CAP_PROP_CONTRAST,CamValues.contrast);
        if (ImGui::SliderFloat("Exposure", &CamValues.exposure, 0.0f, 255.0f)) cap.set(cv::CAP_PROP_EXPOSURE,CamValues.exposure);
    }

    if (cap.isOpened())
    {
        if (cap.grab())
        {
            bind();
            if (cap.retrieve(frame) && !frame.empty())
                setTex(frame);
            else
                setTex(cv::Mat(20, 20, CV_8UC3, cv::Scalar(0, 0, 0)));
        }
    }
}

Window::Window(Window *_new)
{
    *this = *_new;
}
