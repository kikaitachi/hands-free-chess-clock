import audio_capture;
import logger;
import speech_to_text;
import video_capture;

#include <string>

int main() {
  SpeechToText speech_to_text;
  AudioCapture audio_capture(speech_to_text.getSampleRate());
  audio_capture.start([&](const float *samples, int count) {
    std::string result = speech_to_text.infer(samples, count);
    logger::info("Recognised: %s", result.c_str());
  });
  return 0;
}
