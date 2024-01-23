#include "audio_capture.hpp"
#include "command_parser.hpp"
#include "game.hpp"
#include "logger.hpp"
#include "speech_to_text.hpp"
#include "video_capture.hpp"

#include <string>

int main() {
  Game game;
  CommandParser command_parser;
  SpeechToText speech_to_text;
  AudioCapture audio_capture(speech_to_text.getSampleRate(), "plughw:DEV=0,CARD=0");
  VideoCapture video_capture;
  video_capture.start();
  speech_to_text.start([&](const std::string speech) {
    logger::info("Speech: %s", speech.c_str());
    if (command_parser.recognised(speech)) {
      if (!game.playing) {
        game.reset(command_parser.getTime(), command_parser.getIncrement());
      } else {
        logger::info("Not starting new %d+%dms game as there is game in progress",
          command_parser.getTime(), command_parser.getIncrement());
      }
    }
  });
  audio_capture.start([&](const float *samples, int count) {
    speech_to_text.add_audio(samples, count);
  });
  return 0;
}
