import audio_capture;
import video_capture;

int main() {
  AudioCapture audio_capture("hw:0");
  audio_capture.start();
  return 0;
}
