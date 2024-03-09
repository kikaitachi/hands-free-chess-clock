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
