#include "logger.hpp"
#include "video_capture.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <map>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>

#define VIDEO_WIDTH 864
#define VIDEO_HEIGHT 480

using namespace std::chrono_literals;

class Square {
 public:
  std::vector<cv::Point> polygon;
  double center_x, center_y;
  std::optional<Line> topmost_line;
  std::optional<Line> bottommost_line;
  std::optional<Line> leftmost_line;
  std::optional<Line> rightmost_line;
  std::optional<int> row;
  std::optional<int> col;
};

bool square_change_sorter(SquareChange const& lhs, SquareChange const& rhs) {
    return lhs.change > rhs.change;
}

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
    std::function<std::string(SquareChange[64])> on_move_finish
  )
    : on_move_start(on_move_start), on_move_finish(on_move_finish) {
}

void VideoCapture::start() {
  std::thread video_thread(&VideoCapture::capture_frames, this);
  video_thread.detach();
}

static int discover_columns_in_row(
    std::vector<Square>& squares, Square* prev, int start_index, int direction) {
  for (int i = start_index; i < squares.size() && direction == 1 || i >= 0 && direction == -1; i += direction) {
    Square& square = squares[i];
    if (square.row != prev->row) {
      return i;
    }
    double top_line_len = prev->topmost_line->first.x - prev->topmost_line->second.x;
    square.col = prev->col.value() +
      (int)((square.center_x - prev->center_x) / top_line_len);
    prev = &square;
  }
  return -1;
}

static int find_square_bellow_known_column(
    std::vector<Square>& squares, int start_index) {
  for (int i = start_index; i < squares.size(); i++) {
    Square& square = squares[i];
    cv::Rect bounding_box = cv::boundingRect(square.polygon);
    bounding_box.y -= bounding_box.height;
    for (int k = start_index - 1; k >= 0; k--) {
      if (squares[k].center_x > bounding_box.x &&
          squares[k].center_x < bounding_box.x + bounding_box.width &&
          squares[k].center_y > bounding_box.y &&
          squares[k].center_y < bounding_box.y + bounding_box.height) {
        square.col = squares[k].col;
        return i;
      }
    }
  }
  return -1;
}

void VideoCapture::detect_board(cv::Mat& frame, std::string debug_dir) {
  std::filesystem::remove_all(debug_dir);
  std::filesystem::create_directories(debug_dir);
  std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
  cv::imwrite(debug_dir + "/start_game_original.jpg", frame);
  cv::Mat markers;
  frame.copyTo(markers);
  cv::Mat gray;
  cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
  cv::imwrite(debug_dir + "/start_game_gray.jpg", gray);
  cv::Mat blurred;
  cv::medianBlur(gray, blurred, 5);
  cv::imwrite(debug_dir + "/start_game_blurred.jpg", blurred);

  cv::Mat threshold;
  cv::threshold(blurred, threshold, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
  cv::imwrite(debug_dir + "/start_game_threshold.jpg", threshold);
  cv::Mat threshold_inverted;
  cv::threshold(blurred, threshold_inverted, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
  cv::imwrite(debug_dir + "/start_game_threshold_inverted.jpg", threshold_inverted);

  cv::Mat eroded;
  int erosion_size = 3;
  cv::Mat element = getStructuringElement(
    cv::MORPH_RECT,
    cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
    cv::Point(erosion_size, erosion_size));
  cv::erode(threshold, eroded, element);
  cv::imwrite(debug_dir + "/start_game_eroded.jpg", eroded);
  cv::Mat eroded_inverted;
  cv::erode(threshold_inverted, eroded_inverted, element);
  cv::imwrite(debug_dir + "/start_game_eroded_inverted.jpg", eroded_inverted);

  std::vector<std::vector<cv::Point>> contours;
  std::vector<std::vector<cv::Point>> contours_inverted;
  std::vector<cv::Vec4i> hierarchy;

  cv::findContours(eroded, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
  cv::findContours(eroded_inverted, contours_inverted, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
  contours.insert(
    contours.end(),
    std::make_move_iterator(contours_inverted.begin()),
    std::make_move_iterator(contours_inverted.end()));
  cv::Mat img_contours;
  frame.copyTo(img_contours);
  cv::drawContours(img_contours, contours, -1, {0, 0, 255}, 1, cv::LINE_AA);
  cv::imwrite(debug_dir + "/start_game_contours.jpg", img_contours);

  cv::Mat img_polygons;
  frame.copyTo(img_polygons);
  std::vector<Square> squares;
  std::vector<std::vector<cv::Point>> rejected_polygons;

  std::optional<Line> topmost_line;
  std::optional<Line> bottommost_line;
  std::optional<Line> leftmost_line;
  std::optional<Line> rightmost_line;
  for (auto & contour : contours) {
    cv::Rect bounding_box = cv::boundingRect(contour);
    if (bounding_box.height > VIDEO_HEIGHT / 4) {
      continue;  // Reject contours obviously too big to be board cells
    }
    cv::Mat approx;
    approxPolyDP(contour, approx, 10, true);
    if (approx.size().height == 4 && cv::isContourConvex(approx)) {
      std::vector<Line> horizontal_lines;
      std::vector<Line> vertical_lines;
      for (int i = 0; i < 4; i++) {
        cv::Point pt1 = approx.at<cv::Point>(i, 0);
        cv::Point pt2 = approx.at<cv::Point>((i + 1) % 4, 0);
        Line line(pt1, pt2);
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
      if (horizontal_lines.size() == 2) {
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
        cv::polylines(markers, approx, true, {0, 255, 0}, 1, cv::LINE_AA);
        cv::Moments m = cv::moments(contour, false);
        squares.push_back({
          approx,
          m.m10 / m.m00,
          m.m01 / m.m00,
          horizontal_lines[0],
          horizontal_lines[1],
          vertical_lines[0],
          vertical_lines[1]
        });
      } else {
        rejected_polygons.push_back(approx);
      }
    }
  }

  cv::Mat labels;
  std::vector<cv::Point2f> centers_y;
  std::vector<cv::Point2f> centers;
  for (auto & square : squares) {
    centers_y.push_back(cv::Point2f(0, square.center_y));
  }
  cv::kmeans(centers_y, 8, labels,
    cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 10, 1.0),
    3, cv::KMEANS_PP_CENTERS, centers);

  std::vector<int> idx(centers.size());
  std::iota(idx.begin(), idx.end(), 0);
  std::stable_sort(idx.begin(), idx.end(), [&centers](int i1, int i2) {
    return centers[i1].y < centers[i2].y;
  });

  std::vector<cv::Vec2f> top_points, bottom_points, left_points, right_points;

  for (int i = 0; i < squares.size(); i++) {
    Square& square = squares[i];
    cv::polylines(img_polygons, {square.polygon}, true, {0, 255, 0}, 1, cv::LINE_AA);
    for (int j = 0; j < 8; j++) {
      if (labels.at<int>(i) == idx[j]) {
        square.row = j;
        if (j == 0) {
          top_points.push_back(
            cv::Vec2f(
              square.topmost_line.value().first.x,
              square.topmost_line.value().first.y - erosion_size
            ));
          top_points.push_back(
            cv::Vec2f(
              square.topmost_line.value().second.x,
              square.topmost_line.value().second.y - erosion_size
            ));
        } else if (j == 7) {
          bottom_points.push_back(
            cv::Vec2f(
              square.bottommost_line.value().first.x,
              square.bottommost_line.value().first.y + erosion_size
            ));
          bottom_points.push_back(
            cv::Vec2f(
              square.bottommost_line.value().second.x,
              square.bottommost_line.value().second.y + erosion_size
            ));
        }
        break;
      }
    }
  }
  std::stable_sort(squares.begin(), squares.end(), [](const Square& s1, const Square& s2) {
    if (s1.row == s2.row) {
      return s1.center_x < s2.center_x;
    }
    return s1.row < s2.row;
  });
  squares[0].col = 0;
  int next_row = discover_columns_in_row(squares, &squares[0], 1, 1);
  while (next_row != -1) {
    int known_column = find_square_bellow_known_column(squares, next_row);
    if (known_column == -1) {
      break;
    }
    if (known_column > next_row) {  // Fill backwards
      discover_columns_in_row(squares, &squares[known_column], known_column - 1, -1);
    }
    // Fill forward
    next_row = discover_columns_in_row(squares, &squares[known_column], known_column + 1, 1);
  }
  std::map<int, int> col2count;
  for (auto& square : squares) {
    if (square.col.has_value()) {
      col2count[square.col.value()] += 1;
    }
  }
  std::vector<std::pair<int, int>> col_histogram;
  for (auto& [col, count] : col2count) {
    col_histogram.push_back({col, count});
  }
  while (col_histogram.size() > 4) {
    if (col_histogram.front().second < col_histogram.back().second) {
      col_histogram.erase(col_histogram.begin());
    } else {
      col_histogram.erase(col_histogram.end());
    }
  }

  // Draw square labels
  for (int i = 0; i < squares.size(); i++) {
    Square& square = squares[i];
    std::string text = std::to_string(square.row.value());
    bool deleted_column = true;
    if (square.col.has_value()) {
      text += "," + std::to_string(square.col.value());
      for (auto& [col, _] : col_histogram) {
        if (col == square.col.value()) {
          deleted_column = false;
          if (col == col_histogram.front().first) {
            left_points.push_back(cv::Vec2f(
              square.leftmost_line.value().first.x - erosion_size,
              square.leftmost_line.value().first.y
            ));
          } else if (col == col_histogram.back().first) {
            right_points.push_back(cv::Vec2f(
              square.rightmost_line.value().first.x + erosion_size,
              square.rightmost_line.value().first.y
            ));
          }
          break;
        }
      }
    }
    auto font = cv::FONT_HERSHEY_COMPLEX_SMALL;
    cv::Size text_size = cv::getTextSize(text, font, 0.5, 1, 0);
    cv::putText(img_polygons,
      text,
      {
        (int)(square.center_x - text_size.width / 2),
        (int)(square.center_y + text_size.height / 2)
      },
      font,
      0.5,
      {0, 255, deleted_column ? 255.0 : 0.0},
      1,
      cv::LINE_AA);
  }

  cv::Vec4f top_line;
  cv::fitLine(top_points, top_line, cv::DIST_L2, 0, 0.01, 0.01);
  double vx = top_line[0];
  double vy = top_line[1];
  double x0 = top_line[2];
  double y0 = top_line[3];
  double m = 1000;
  topmost_line = {
    {(int)(x0 - m * vx), (int)(y0 - m * vy)},
    {(int)(x0 + m * vx), (int)(y0 + m * vy)}
  };
  cv::line(markers,
    topmost_line.value().first, topmost_line.value().second,
    {255, 0, 0}, 1, cv::LINE_AA);

  cv::Vec4f bottom_line;
  cv::fitLine(bottom_points, bottom_line, cv::DIST_L2, 0, 0.01, 0.01);
  vx = bottom_line[0];
  vy = bottom_line[1];
  x0 = bottom_line[2];
  y0 = bottom_line[3];
  bottommost_line = {
    {(int)(x0 - m * vx), (int)(y0 - m * vy)},
    {(int)(x0 + m * vx), (int)(y0 + m * vy)}
  };
  cv::line(markers,
    bottommost_line.value().first, bottommost_line.value().second,
    {255, 0, 0}, 1, cv::LINE_AA);

  cv::Vec4f left_line;
  cv::fitLine(left_points, left_line, cv::DIST_L2, 0, 0.01, 0.01);
  vx = left_line[0];
  vy = left_line[1];
  x0 = left_line[2];
  y0 = left_line[3];
  leftmost_line = {
    {(int)(x0 - m * vx), (int)(y0 - m * vy)},
    {(int)(x0 + m * vx), (int)(y0 + m * vy)}
  };
  cv::line(markers,
    leftmost_line.value().first, leftmost_line.value().second,
    {255, 0, 0}, 1, cv::LINE_AA);

  cv::Vec4f right_line;
  cv::fitLine(right_points, right_line, cv::DIST_L2, 0, 0.01, 0.01);
  vx = right_line[0];
  vy = right_line[1];
  x0 = right_line[2];
  y0 = right_line[3];
  rightmost_line = {
    {(int)(x0 - m * vx), (int)(y0 - m * vy)},
    {(int)(x0 + m * vx), (int)(y0 + m * vy)}
  };
  cv::line(markers,
    rightmost_line.value().first, rightmost_line.value().second,
    {255, 0, 0}, 1, cv::LINE_AA);

  for (auto & polygon : rejected_polygons) {
    cv::polylines(img_polygons, {polygon}, true, {0, 0, 255}, 1, cv::LINE_AA);
  }
  cv::imwrite(debug_dir + "/start_game_polygons.jpg", img_polygons);

  cv::Point bottom_left_point = line_intersection(leftmost_line.value(), bottommost_line.value());
  cv::circle(markers, bottom_left_point, 8, {0, 0, 255}, -1);
  cv::Point bottom_right_point = line_intersection(rightmost_line.value(), bottommost_line.value());
  cv::circle(markers, bottom_right_point, 8, {0, 0, 255}, -1);
  cv::Point top_left_point = line_intersection(leftmost_line.value(), topmost_line.value());
  cv::circle(markers, top_left_point, 8, {0, 0, 255}, -1);
  cv::Point top_right_point = line_intersection(rightmost_line.value(), topmost_line.value());
  cv::circle(markers, top_right_point, 8, {0, 0, 255}, -1);

  int distance_bottom_x = bottom_left_point.x - bottom_right_point.x;
  int distance_bottom_y = bottom_left_point.y - bottom_right_point.y;
  cv::Point final_bottom_left_point(
    bottom_left_point.x + distance_bottom_x / 2,
    bottom_left_point.y + distance_bottom_y / 2);
  cv::circle(markers, final_bottom_left_point, 8, {255, 0, 255}, -1);
  cv::Point final_bottom_right_point(
    bottom_right_point.x - distance_bottom_x / 2,
    bottom_right_point.y - distance_bottom_y / 2);
  cv::circle(markers, final_bottom_right_point, 8, {255, 0, 255}, -1);

  int distance_top_x = top_left_point.x - top_right_point.x;
  int distance_top_y = top_left_point.y - top_right_point.y;
  cv::Point final_top_left_point(
    top_left_point.x + distance_top_x / 2,
    top_left_point.y + distance_top_y / 2);
  cv::circle(markers, final_top_left_point, 8, {255, 0, 255}, -1);
  cv::Point final_top_right_point(
    top_right_point.x - distance_top_x / 2,
    top_right_point.y - distance_top_y / 2);
  cv::circle(markers, final_top_right_point, 8, {255, 0, 255}, -1);

  cv::imwrite(debug_dir + "/start_game_markers.jpg", markers);

  std::vector<cv::Point2f> points_from({
    final_bottom_left_point,
    final_top_left_point,
    final_top_right_point,
    final_bottom_right_point
  });
  std::vector<cv::Point2f> points_to({
    {0.0f, VIDEO_HEIGHT},
    {0.0f, 0.0f},
    {VIDEO_HEIGHT, 0.0f},
    {VIDEO_HEIGHT, VIDEO_HEIGHT}
  });
  perspective_transform = cv::getPerspectiveTransform(points_from, points_to);

  cv::warpPerspective(
      frame, img_perspective, perspective_transform,
      {VIDEO_HEIGHT, VIDEO_HEIGHT}
  );
  cv::imwrite(debug_dir + "/start_game_perspective.jpg", img_perspective);
  std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
  logger::info("Detected chess board in %dms",
    (int)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
}

void VideoCapture::start_game() {
  std::lock_guard<std::mutex> guard(frame_mutex);

  detect_board(frame, "debug");

  cv::cvtColor(img_perspective, last_move, cv::COLOR_BGR2GRAY);

  bg_sub = cv::createBackgroundSubtractorMOG2(500, 32, true);
  // Ensure that there are no changes in the inital frames
  cv::Mat mask;
  bg_sub->apply(img_perspective, mask, 1);

  ply_index = 1;
}

void VideoCapture::resume_game() {
  std::lock_guard<std::mutex> guard(frame_mutex);
  cv::warpPerspective(
      frame, img_perspective, perspective_transform,
      {VIDEO_HEIGHT, VIDEO_HEIGHT}
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
  cap.open(deviceID, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    logger::error("Failed to open camera");
    return;
  }
  cap.set(cv::CAP_PROP_FRAME_WIDTH, VIDEO_WIDTH);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, VIDEO_HEIGHT);
  for (; ; ) {
    frame_mutex.lock();
    for ( ; ; ) {
      cap.read(frame);
      cv::Mat laplacian;
      cv::Laplacian(frame, laplacian, CV_64F);
      cv::Scalar mean, stddev;
      cv::meanStdDev(laplacian, mean, stddev);
      double variance = stddev.val[0] * stddev.val[0];
      if (variance < 80) {
        cv::imwrite("debug/blurry.jpg", frame);
        logger::info("Rejecting blurry image with variance: %f", variance);
      } else {
        break;
      }
    }
    if (!bg_sub.empty()) {
      cv::warpPerspective(
          frame, img_perspective, perspective_transform,
          {VIDEO_HEIGHT, VIDEO_HEIGHT}
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
          cv::Mat blurred;
          cv::medianBlur(diff, blurred, 5);
          cv::adaptiveThreshold(blurred, diff, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 5, 2);
          cv::Mat without_cell_boundaries = cv::Mat::zeros(
            cv::Size(VIDEO_HEIGHT, VIDEO_HEIGHT), diff.type());
          int square_size = VIDEO_HEIGHT / 8;
          int margin = 10;
          for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
              cv::Rect cell = cv::Rect(
                x * square_size + margin / 2, y * square_size + margin / 2,
                square_size - margin, square_size - margin);
              diff(cell).copyTo(without_cell_boundaries(cell));
            }
          }
          without_cell_boundaries.copyTo(diff);

          SquareChange changes[64];
          for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
              cv::Mat square = diff(cv::Rect(x * square_size, y * square_size, square_size, square_size));
              double sum = cv::sum(square).dot(cv::Scalar::ones());
              changes[y * 8 + x] = {x, y, (7 - x) * 8 + (7 - y), sum};
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

          std::string move = on_move_finish(changes);
          std::string move_number = std::to_string(ply_index);
          move_number.insert(move_number.begin(), 3 - move_number.size(), '0');
          if (!move.empty()) {
            save_differences(img_perspective, colored,
              "debug/move" + move_number + "-" + move + ".jpg");
            last_move = img_perspective.clone();
            cv::cvtColor(img_perspective, last_move, cv::COLOR_BGR2GRAY);
            if (move == "take back") {
              ply_index--;
            } else {
              ply_index++;
            }
          } else {
            save_differences(img_perspective, colored,
              "debug/move" + move_number + "-failed.jpg");
          }
        }
      } else if (total_changes > 2000000) {
        moving = true;
        on_move_start();
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

void VideoCapture::save_differences(cv::Mat& img_perspective, cv::Mat& colored, std::string file_name) {
  cv::Mat bg_sub;
  cv::Mat images[] = {
    img_perspective, colored
  };
  cv::hconcat(images, 2, bg_sub);
  cv::imwrite(file_name, bg_sub);
  cv::cvtColor(img_perspective, last_move, cv::COLOR_BGR2GRAY);
}
