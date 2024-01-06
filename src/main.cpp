import audio_capture;
import command_parser;
import logger;
import speech_to_text;
import video_capture;

#include <string>

int main() {
  CommandParser command_parser;
  SpeechToText speech_to_text;
  AudioCapture audio_capture(speech_to_text.getSampleRate());
  audio_capture.start([&](const float *samples, int count) {
    std::string speech = speech_to_text.infer(samples, count);
    logger::info("Speech: %s", speech.c_str());
    if (command_parser.recognised(speech)) {
      logger::info("Set chess clock to: %d+%dms",
        command_parser.getTime(), command_parser.getIncrement());
    }
  });
  return 0;
}
