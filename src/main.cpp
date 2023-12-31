import audio_capture;
import speech_to_text;
import video_capture;

int main() {
  AudioCapture audio_capture("hw:0");
  SpeechToText speech_to_text;
  audio_capture.start();
  return 0;
}
