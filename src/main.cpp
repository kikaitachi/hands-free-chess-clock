import audio_capture;
import speech_to_text;
import video_capture;

#include <unistd.h>

int main() {
  SpeechToText speech_to_text;
  AudioCapture audio_capture(speech_to_text.getSampleRate());
  audio_capture.start([&](const float *samples, int count) {
    speech_to_text.infer(samples, count);
    //write(1, samples, count * sizeof(float));
  });
  return 0;
}
