#include "video_capture.hpp"
#include <opencv2/opencv.hpp>
#include "logger.hpp"
#include <thread>

void VideoCapture::start() {
  std::thread video_thread(&VideoCapture::process, this);
  video_thread.detach();
}

void VideoCapture::process() {
  cv::Mat frame;
  cv::VideoCapture cap;
  int deviceID = 0;             // 0 = open default camera
  int apiID = cv::CAP_ANY;      // 0 = autodetect default API
  cap.open(deviceID, apiID);
  if (!cap.isOpened()) {
    logger::error("Failed to open camera");
    return;
  }
  for (int i = 0; ; i++) {
    cap.read(frame);
    if (frame.empty()) {
      logger::error("Blank frame grabbed");
    }
    std::string file_name = "test" + std::to_string(i) + ".jpg";
    cv::imwrite(file_name, frame);
  }
}
