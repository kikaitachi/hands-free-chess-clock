#include "video_capture.hpp"
#include <opencv2/opencv.hpp>
#include "logger.hpp"

void VideoCapture::start() {
  cv::Mat frame;
  cv::VideoCapture cap;
  int deviceID = 0;             // 0 = open default camera
  int apiID = cv::CAP_ANY;      // 0 = autodetect default API
  cap.open(deviceID, apiID);
  if (!cap.isOpened()) {
    logger::error("Failed to open camera");
    return;
  }
  cap.read(frame);
  if (frame.empty()) {
    logger::error("Blank frame grabbed");
  }
  cv::imwrite("test2.jpg", frame);
}
