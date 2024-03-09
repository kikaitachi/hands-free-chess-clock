#include <gtest/gtest.h>
#include "../src/video_capture.hpp"

TEST(VideoCaptureTest, EnoughSquaresDetected) {
  VideoCapture video_capture(
    [&]() {
    },
    [&](SquareChange changes[64]) {
      return "";
    }
  );

  // TODO: test
}
