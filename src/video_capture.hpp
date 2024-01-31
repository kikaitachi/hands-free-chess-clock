#ifndef VIDEO_CAPTURE_H_
#define VIDEO_CAPTURE_H_

#include <functional>
#include <mutex>
#include <opencv2/opencv.hpp>

class VideoCapture {
 public:
  VideoCapture(
    std::function<void()> on_move_start,
    std::function<void()> on_move_finish
  );
  void start_game();
  void resume_game();
  void stop_game();

 private:
  cv::Mat frame;
  cv::Mat perspective_transform;
  cv::Ptr<cv::BackgroundSubtractor> bg_sub;
  std::mutex frame_mutex;
  std::function<void()> on_move_start;
  std::function<void()> on_move_finish;
  bool moving = false;

  void capture_frames();
};


#endif  // VIDEO_CAPTURE_H_
