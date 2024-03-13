#include <gtest/gtest.h>
#include "../src/video_capture.hpp"
#include <opencv2/opencv.hpp>

TEST(VideoCaptureTest, BigWoodenBoard) {
  VideoCapture video_capture(
    [&]() {
    },
    [&](SquareChange changes[64]) {
      return "";
    }
  );
  cv::Mat frame = cv::imread("../test/boards/big-wooden.jpg");
  video_capture.detect_board(frame, "big-wooden");
}

TEST(VideoCaptureTest, SmallMagneticFlatPiecesBoard) {
  VideoCapture video_capture(
    [&]() {
    },
    [&](SquareChange changes[64]) {
      return "";
    }
  );
  cv::Mat frame = cv::imread("../test/boards/small-magnetic-flat-pieces.jpg");
  video_capture.detect_board(frame, "small-magnetic-flat-pieces");
}

TEST(VideoCaptureTest, Difference) {
  std::vector<std::string> file_names({
    "move000.jpg",
    "move001-d2d4.jpg",
    "move002-b7b5.jpg",
    "move003-e2e4.jpg",
    "move004-g8f6.jpg",
    "move005-g1f3.jpg",
    "move006-c8a6.jpg",
    "move007-f1b5.jpg",
    "move008-a6b5.jpg",
    "move009-b1c3.jpg",
    "move010-g6f4.jpg"
  });
  cv::Mat before = cv::imread("../test/boards/" + file_names[0]);
  cv::Mat hsv_before;
  cv::cvtColor(before, hsv_before, cv::COLOR_BGR2HSV);
  std::vector<cv::Mat> hsv_planes_before;
  cv::split(hsv_before, hsv_planes_before);
  for (int i = 1; i < file_names.size(); i++) {
    cv::Mat after = cv::imread("../test/boards/" + file_names[i]);
    cv::Mat hsv_after;
    cv::cvtColor(after, hsv_after, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> hsv_planes_after;
    cv::split(hsv_after, hsv_planes_after);
    cv::Mat diff;
    cv::Mat diff_hue;
    cv::absdiff(before, after, diff);
    cv::absdiff(hsv_planes_before[0], hsv_planes_after[0], diff_hue);
    cv::imwrite("absdiff" + std::to_string(i) + ".jpg", diff);
    cv::imwrite("diff_hue" + std::to_string(i) + ".jpg", diff_hue);

    cv::Mat diff_gray;
    cv::cvtColor(diff, diff_gray, cv::COLOR_BGR2GRAY);
    cv::imwrite("diff_gray" + std::to_string(i) + ".jpg", diff_gray);

    cv::Mat diff_threshold;
    cv::threshold(diff_gray, diff_threshold, 32, 255, cv::THRESH_BINARY);
    cv::imwrite("diff_threshold" + std::to_string(i) + ".jpg", diff_threshold);

    cv::Mat diff_otsu;
    cv::threshold(diff_gray, diff_otsu, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    cv::imwrite("diff_otsu" + std::to_string(i) + ".jpg", diff_otsu);

    before = after;
    hsv_planes_before[0] = hsv_planes_after[0];
  }
}
