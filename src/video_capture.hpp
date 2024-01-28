#ifndef VIDEO_CAPTURE_H_
#define VIDEO_CAPTURE_H_

#include <mutex>
#include <opencv2/opencv.hpp>

class VideoCapture {
 public:
  VideoCapture();
  void start_game();

 private:
  cv::Mat frame;
  cv::Mat perspective_transform;
  cv::Ptr<cv::BackgroundSubtractor> bg_sub;
  std::mutex frame_mutex;

  void capture_frames();
};


#endif  // VIDEO_CAPTURE_H_
