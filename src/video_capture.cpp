#include "logger.hpp"
#include "video_capture.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>

#define WIDTH 864
#define HEIGHT 480

using namespace std::chrono_literals;

bool square_change_sorter(SquareChange const& lhs, SquareChange const& rhs) {
    return lhs.change > rhs.change;
}

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

VideoCapture::VideoCapture(
    std::function<void()> on_move_start,
    std::function<bool(SquareChange[64])> on_move_finish
  )
    : on_move_start(on_move_start), on_move_finish(on_move_finish) {
  // Always keep video capture running, otherwise camera will not be focused
  // for a few frames on startup
  std::thread video_thread(&VideoCapture::capture_frames, this);
  video_thread.detach();
}

void VideoCapture::start_game() {
  std::filesystem::remove_all("debug");
  std::filesystem::create_directories("debug");
  // Detect board
  std::lock_guard<std::mutex> guard(frame_mutex);
  cv::imwrite("debug/start_game_original.jpg", frame);
  cv::Mat markers;
  frame.copyTo(markers);
  cv::Mat gray;
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  cv::imwrite("debug/start_game_gray.jpg", gray);
  cv::Mat blurred;
  cv::medianBlur(gray, blurred, 5);
  cv::imwrite("debug/start_game_blurred.jpg", blurred);
  cv::Mat threshold;
  cv::adaptiveThreshold(blurred, threshold, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 2);
  cv::imwrite("debug/start_game_threshold.jpg", threshold);
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
  cv::circle(markers, bottom_left_point, 10, {0, 0, 255}, -1);
  cv::Point bottom_right_point = line_intersection(rightmost_line.value(), bottommost_line.value());
  cv::circle(markers, bottom_right_point, 10, {0, 0, 255}, -1);
  cv::Point top_left_point = line_intersection(leftmost_line.value(), topmost_line.value());
  cv::circle(markers, top_left_point, 10, {0, 0, 255}, -1);
  cv::Point top_right_point = line_intersection(rightmost_line.value(), topmost_line.value());
  cv::circle(markers, top_right_point, 10, {0, 0, 255}, -1);

  int distance_bottom = bottom_left_point.x - bottom_right_point.x;
  cv::Point final_bottom_left_point(bottom_left_point.x + distance_bottom / 2, bottom_left_point.y);
  cv::circle(markers, final_bottom_left_point, 10, {255, 0, 255}, -1);
  cv::Point final_bottom_right_point(bottom_right_point.x - distance_bottom / 2, bottom_left_point.y);
  cv::circle(markers, final_bottom_right_point, 10, {255, 0, 255}, -1);

  int distance_top = top_left_point.x - top_right_point.x;
  cv::Point final_top_left_point(top_left_point.x + distance_top / 2, top_left_point.y);
  cv::circle(markers, final_top_left_point, 10, {255, 0, 255}, -1);
  cv::Point final_top_right_point(top_right_point.x - distance_top / 2, top_left_point.y);
  cv::circle(markers, final_top_right_point, 10, {255, 0, 255}, -1);

  cv::imwrite("debug/start_game_markers.jpg", markers);

  std::vector<cv::Point2f> points_from({
    final_bottom_left_point,
    final_top_left_point,
    final_top_right_point,
    final_bottom_right_point
  });
  float height = markers.size().height;
  std::vector<cv::Point2f> points_to({
    {0.0f, height},
    {0.0f, 0.0f},
    {height, 0.0f},
    {height, height}
  });
  perspective_transform = cv::getPerspectiveTransform(points_from, points_to);

  cv::Mat img_perspective;
  cv::warpPerspective(
      frame, img_perspective, perspective_transform,
      {(int)height, (int)height}
  );
  cv::imwrite("debug/start_game_perspective.jpg", img_perspective);

  cv::cvtColor(img_perspective, last_move, cv::COLOR_BGR2GRAY);

  bg_sub = cv::createBackgroundSubtractorMOG2(500, 32, true);
  // Ensure that there are no changes in the inital frames
  cv::Mat mask;
  bg_sub->apply(img_perspective, mask, 1);
}

void VideoCapture::resume_game() {
  std::lock_guard<std::mutex> guard(frame_mutex);
  float height = frame.size().height;
  cv::Mat img_perspective;
  cv::warpPerspective(
      frame, img_perspective, perspective_transform,
      {(int)height, (int)height}
  );
  bg_sub = cv::createBackgroundSubtractorMOG2(500, 32, true);
  // Ensure that there are no changes in the inital frames
  cv::Mat mask;
  bg_sub->apply(img_perspective, mask, 1);
}

void VideoCapture::stop_game() {
  std::lock_guard<std::mutex> guard(frame_mutex);
  bg_sub.reset();
}

void VideoCapture::capture_frames() {
  cv::VideoCapture cap;
  int deviceID = 0;  // 0 = open default camera
  cap.open(deviceID);
  if (!cap.isOpened()) {
    logger::error("Failed to open camera");
    return;
  }
  cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
  for (int i = 1; ; ) {
    frame_mutex.lock();
    cap.read(frame);
    if (!bg_sub.empty()) {
      int height = frame.size().height;
      cv::Mat img_perspective;
      cv::warpPerspective(
          frame, img_perspective, perspective_transform,
          {height, height}
      );
      cv::Mat mask;
      bg_sub->apply(img_perspective, mask, -1);
      double total_changes = cv::sum(mask).dot(cv::Scalar::ones());
      //logger::debug("Background subtraction change: %f", total_changes);
      if (moving) {
        if (total_changes < 1500000) {
          moving = false;

          cv::Mat gray_perspective;
          cv::cvtColor(img_perspective, gray_perspective, cv::COLOR_BGR2GRAY);
          cv::Mat diff;
          cv::absdiff(last_move, gray_perspective, diff);

          int square_size = HEIGHT / 8;
          SquareChange changes[64];
          for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
              cv::Mat square = diff(cv::Rect(x * square_size, y * square_size, square_size, square_size));
              double sum = cv::sum(square).dot(cv::Scalar::ones());
              changes[y * 8 + x] = {x, y, sum};
            }
          }
          std::sort(changes, changes + 64, &square_change_sorter);

          cv::Mat colored;
          cv::cvtColor(diff, colored, cv::COLOR_GRAY2BGR);

          for (int j = 0; j < 6; j++) {
            SquareChange change = changes[j];
            cv::rectangle(colored,
              {change.x * square_size, change.y * square_size},
              {change.x * square_size + square_size, change.y * square_size + square_size},
              {0, 0, 255}, 1, cv::LINE_AA);
          }

          if (on_move_finish(changes)) {
            cv::Mat bg_sub;
            cv::Mat images[] = {
              img_perspective, colored
            };
            cv::hconcat(images, 2, bg_sub);
            std::string move_number = std::to_string(i);
            move_number.insert(move_number.begin(), 3 - move_number.size(), '0');
            cv::imwrite("debug/move" + move_number + ".jpg", bg_sub);
            last_move = img_perspective.clone();
            cv::cvtColor(img_perspective, last_move, cv::COLOR_BGR2GRAY);
            i++;
          }
        }
      } else {
        if (total_changes > 2000000) {
          moving = true;
          on_move_start();
        }
      }
    }
    frame_mutex.unlock();
    if (frame.empty()) {
      logger::warn("Blank frame grabbed");
    }
    // Increase chance for start_game to get the lock
    std::this_thread::sleep_for(1ms);
  }
}
