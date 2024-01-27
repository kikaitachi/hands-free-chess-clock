#include "logger.hpp"
#include "video_capture.hpp"
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <thread>

VideoCapture::VideoCapture() {
  std::filesystem::create_directories("images");
  // Always keep video capture running, otherwise camera will not be focused
  // for a few frames on startup
  std::thread video_thread(&VideoCapture::capture_frames, this);
  video_thread.detach();
}

void VideoCapture::start_game() {
  // Detect board
  std::lock_guard<std::mutex> guard(frame_mutex);
  cv::imwrite("images/start_game_original.jpg", frame);
  // TODO: implement
}

void VideoCapture::capture_frames() {
  cv::VideoCapture cap;
  int deviceID = 0;             // 0 = open default camera
  int apiID = cv::CAP_ANY;      // 0 = autodetect default API
  cap.open(deviceID, apiID);
  if (!cap.isOpened()) {
    logger::error("Failed to open camera");
    return;
  }
  for (int i = 0; ; i++) {
    frame_mutex.lock()
    cap.read(frame);
    frame_mutex.unlock();
    if (frame.empty()) {
      logger::error("Blank frame grabbed");
    }
  }
}
