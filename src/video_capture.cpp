#include "logger.hpp"
#include "video_capture.hpp"
#include <filesystem>
#include <thread>

using namespace std::chrono_literals;

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
  cv::Mat gray;
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  cv::imwrite("images/start_game_gray.jpg", gray);
  cv::Mat blurred;
  cv::medianBlur(gray, blurred, 5);
  cv::imwrite("images/start_game_blurred.jpg", blurred);
  cv::Mat threshold;
  cv::adaptiveThreshold(blurred, threshold, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 2);
  cv::imwrite("images/start_game_threshold.jpg", threshold);
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
  for ( ; ; ) {
    frame_mutex.lock();
    cap.read(frame);
    frame_mutex.unlock();
    if (frame.empty()) {
      logger::warn("Blank frame grabbed");
    }
    // Increase chance for start_game to get the lock
    std::this_thread::sleep_for(2ms);
  }
}
