#include "logger.hpp"
#include "video_capture.hpp"
#include <cmath>
#include <filesystem>
#include <thread>
#include <utility>

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
  cv::Mat markers(frame);
  cv::Mat gray;
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  cv::imwrite("images/start_game_gray.jpg", gray);
  cv::Mat blurred;
  cv::medianBlur(gray, blurred, 5);
  cv::imwrite("images/start_game_blurred.jpg", blurred);
  cv::Mat threshold;
  cv::adaptiveThreshold(blurred, threshold, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 2);
  cv::imwrite("images/start_game_threshold.jpg", threshold);
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(threshold, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
  for (auto & contour : contours) {
    cv::Mat approx;
    approxPolyDP(contour, approx, 20, true);
    if (approx.size().height == 4 && cv::isContourConvex(approx)) {
      std::vector<std::pair<cv::Point, cv::Point>> horizontal_lines;
      std::vector<std::pair<cv::Point, cv::Point>> vertical_lines;
      std::vector<double> line_lengths;
      for (int i = 0; i < 4; i++) {
        cv::Point pt1 = approx.at<cv::Point>(i, 0);
        cv::Point pt2 = approx.at<cv::Point>((i + 1) % 4, 0);
        std::pair<cv::Point, cv::Point> line;
        //cv::line(markers, pt1, pt2, {0, 255, 0}, 5, cv::LINE_AA);
        line_lengths.push_back(std::sqrt(std::pow(pt2.y - pt1.y, 2) + std::pow(pt2.x - pt1.y, 2)));
        double angle = std::atan2(pt2.y - pt1.y, pt2.x - pt1.x) * 180.0 / M_PI;
        if (
            angle > -5 && angle < 5 ||
            angle > -185 && angle < -175 ||
            angle > 175 && angle < 185
        ) {
          horizontal_lines.push_back(line);
        } else {
          vertical_lines.push_back(line);
        }
      }
      std::sort(line_lengths.begin(), line_lengths.end());
      if (horizontal_lines.size() == 2 && line_lengths[0] > line_lengths[3] * 0.45) {
        cv::polylines(markers, approx, true, {0, 255, 0}, 5, cv::LINE_AA);
      }
    }
  }
  cv::imwrite("images/start_game_markers.jpg", markers);
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
