#ifndef VIDEO_CAPTURE_H_
#define VIDEO_CAPTURE_H_

#include <functional>
#include <mutex>
#include <opencv2/opencv.hpp>

typedef std::pair<cv::Point, cv::Point> Line;

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

  void start();

  void detect_board(cv::Mat& frame, std::string debug_dir);

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
  cv::Mat img_perspective;
  cv::Mat last_move;
  cv::Ptr<cv::BackgroundSubtractor> bg_sub;
  std::mutex frame_mutex;
  std::function<void()> on_move_start;
  std::function<std::string(SquareChange[64])> on_move_finish;
  bool moving = false;
  int ply_index;

  void capture_frames();
  void save_differences(cv::Mat& img_perspective, cv::Mat& colored, std::string file_name);
};


#endif  // VIDEO_CAPTURE_H_
