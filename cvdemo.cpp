#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>
using namespace cv;

#include <iostream>

int main()
{
    std::cout << "OpenCV version : " << CV_VERSION << std::endl;
    std::cout << "Major version : " << CV_MAJOR_VERSION << std::endl;
    std::cout << "Minor version : " << CV_MINOR_VERSION << std::endl;
    std::cout << "Subminor version : " << CV_SUBMINOR_VERSION << std::endl;

    // Or using the function
    std::cout << "OpenCV Version: " << cv::getVersionString() << std::endl;

  cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

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
