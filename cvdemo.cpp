#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
using namespace cv;

#include <iostream>

int main()
{
    VideoCapture cap;
    int deviceID = 0;
    int apiID = cv::CAP_ANY;

    cap.open(deviceID, apiID);

    UMat frame;

    if (!cap.isOpened())
    {
        std::cerr << "ERROR! Unable to open camera" << std::endl;
        return -1;
    }

    for (;;)
    {
        cap.read(frame);
        if (frame.empty())
        {
            continue;
        }

        imshow("Live",frame);
        if (waitKey(5) >= 0)
            break;
    }

    return 0;
}