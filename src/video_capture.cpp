#include "logger.hpp"
#include "video_capture.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>

using namespace std::chrono_literals;

typedef std::pair<cv::Point, cv::Point> Line;

struct {
  bool operator()(Line a, Line b) const {
    return std::min(a.first.y, a.second.y) < std::min(b.first.y, b.second.y);
  }
}
line_top;

struct {
  bool operator()(Line a, Line b) const {
    return std::max(a.first.y, a.second.y) < std::max(b.first.y, b.second.y);
  }
}
line_bottom;

struct {
  bool operator()(Line a, Line b) const {
    return std::min(a.first.x, a.second.x) < std::min(b.first.x, b.second.x);
  }
}
line_left;

struct {
  bool operator()(Line a, Line b) const {
    return std::max(a.first.x, a.second.x) < std::max(b.first.x, b.second.x);
  }
}
line_right;

double det(double ax, double ay, double bx, double by) {
  return ax * by - ay * bx;
}

cv::Point line_intersection(Line line1, Line line2) {
  double ax = line1.first.x - line1.second.x;
  double ay = line2.first.x - line2.second.x;
  double bx = line1.first.y - line1.second.y;
  double by = line2.first.y - line2.second.y;

  double div = det(ax, ay, bx, by);
  if (div == 0) {
    throw std::runtime_error("lines do not intersect");
  }

  double cx = det(line1.first.x, line1.first.y, line1.second.x, line1.second.y);
  double cy = det(line2.first.x, line2.first.y, line2.second.x, line2.second.y);
  double x = det(cx, cy, ax, ay) / div;
  double y = det(cx, cy, bx, by) / div;
  return cv::Point((int)x, (int)y);
}

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
  std::optional<Line> topmost_line;
  std::optional<Line> bottommost_line;
  std::optional<Line> leftmost_line;
  std::optional<Line> rightmost_line;
  for (auto & contour : contours) {
    cv::Mat approx;
    approxPolyDP(contour, approx, 20, true);
    if (approx.size().height == 4 && cv::isContourConvex(approx)) {
      std::vector<Line> horizontal_lines;
      std::vector<Line> vertical_lines;
      std::vector<double> line_lengths;
      for (int i = 0; i < 4; i++) {
        cv::Point pt1 = approx.at<cv::Point>(i, 0);
        cv::Point pt2 = approx.at<cv::Point>((i + 1) % 4, 0);
        Line line(pt1, pt2);
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
        std::sort(horizontal_lines.begin(), horizontal_lines.end(), line_top);
        if (!topmost_line || !line_top.operator()(topmost_line.value(), horizontal_lines[0])) {
          topmost_line = horizontal_lines[0];
        }
        std::sort(horizontal_lines.begin(), horizontal_lines.end(), line_bottom);
        if (!bottommost_line || line_bottom.operator()(bottommost_line.value(), horizontal_lines[1])) {
          bottommost_line = horizontal_lines[1];
        }
        std::sort(vertical_lines.begin(), vertical_lines.end(), line_left);
        if (!leftmost_line || !line_left.operator()(leftmost_line.value(), vertical_lines[0])) {
          leftmost_line = vertical_lines[0];
        }
        std::sort(vertical_lines.begin(), vertical_lines.end(), line_right);
        if (!rightmost_line || line_right.operator()(rightmost_line.value(), vertical_lines[1])) {
          rightmost_line = vertical_lines[1];
        }
        cv::polylines(markers, approx, true, {0, 255, 0}, 5, cv::LINE_AA);
      }
    }
  }
  cv::line(markers, topmost_line.value().first, topmost_line.value().second, {0, 0, 255}, 5, cv::LINE_AA);
  cv::line(markers, bottommost_line.value().first, bottommost_line.value().second, {0, 0, 255}, 5, cv::LINE_AA);
  cv::line(markers, leftmost_line.value().first, leftmost_line.value().second, {0, 0, 255}, 5, cv::LINE_AA);
  cv::line(markers, rightmost_line.value().first, rightmost_line.value().second, {0, 0, 255}, 5, cv::LINE_AA);

  cv::Point bottom_left_point = line_intersection(leftmost_line.value(), bottommost_line.value());
  cv::circle(markers, bottom_left_point, 10, (0, 0, 255), -1);
  cv::Point bottom_right_point = line_intersection(rightmost_line.value(), bottommost_line.value());
  cv::circle(markers, bottom_right_point, 10, (0, 0, 255), -1);
  cv::Point top_left_point = line_intersection(leftmost_line.value(), topmost_line.value());
  cv::circle(markers, top_left_point, 10, (0, 0, 255), -1);
  cv::Point top_right_point = line_intersection(rightmost_line.value(), topmost_line.value());
  cv::circle(markers, top_right_point, 10, (0, 0, 255), -1);

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
