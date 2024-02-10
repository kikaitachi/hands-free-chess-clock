#ifndef VIDEO_CAPTURE_H_
#define VIDEO_CAPTURE_H_

#include <functional>
#include <mutex>
#include <opencv2/opencv.hpp>

/**
 * How much given square changes between last move and suspected new move.
 */
class SquareChange {
 public:
  int x;
  int y;
  int index;
  double change;
};

class VideoCapture {
 public:
  VideoCapture(
    std::function<void()> on_move_start,
    std::function<std::string(SquareChange[64])> on_move_finish
  );

  /**
   * Called when game starts. Assumption is that all pieces are on their
   * initial squares.
   */
  void start_game();

  /**
   * Resume stopped game.
   */
  void resume_game();

  /**
   * Stop game either because it finished or because of user request.
   */
  void stop_game();

 private:
  cv::Mat frame;
  cv::Mat perspective_transform;
  cv::Mat last_move;
  cv::Ptr<cv::BackgroundSubtractor> bg_sub;
  std::mutex frame_mutex;
  std::function<void()> on_move_start;
  std::function<std::string(SquareChange[64])> on_move_finish;
  bool moving = false;

  void capture_frames();
};


#endif  // VIDEO_CAPTURE_H_
