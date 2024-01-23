#include "audio_capture.hpp"
#include "command_parser.hpp"
#include "display.hpp"
#include "logger.hpp"
#include "speech_to_text.hpp"
#include "video_capture.hpp"

#include <string>

int main() {
  Display display;
  CommandParser command_parser;
  SpeechToText speech_to_text;
  AudioCapture audio_capture(speech_to_text.getSampleRate(), "plughw:DEV=0,CARD=0");
  VideoCapture video_capture;
  video_capture.start();
  speech_to_text.start([&](const std::string speech) {
    logger::info("Speech: %s", speech.c_str());
    if (command_parser.recognised(speech)) {
      logger::info("Set chess clock to: %d+%dms",
        command_parser.getTime(), command_parser.getIncrement());
      display.set_white("test");
    }
  });
  display.set_white("d2d4");
  display.set_black("10:2.5");
  audio_capture.start([&](const float *samples, int count) {
    speech_to_text.add_audio(samples, count);
  });
  return 0;
}
