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
  AudioCapture audio_capture(speech_to_text.getSampleRate(), "plughw:DEV=0,CARD=C920");
  VideoCapture video_capture(
    [&]() {
      logger::info("Move started");
    },
    [&]() {
      logger::info("Move finished");
      game.switch_clock();
    }
  );
  speech_to_text.start(
    [&]() {
      game.ready();
    },
    [&](const std::string speech) {
      logger::info("Speech: %s", speech.c_str());
      switch (command_parser.recognised(speech)) {
        case START_GAME:
          if (!game.playing) {
            video_capture.start_game();
            game.start(
              command_parser.getTime(), command_parser.getIncrement(),
              [&]() {
                video_capture.stop_game();
              }
            );
          } else {
            logger::info("Not starting new %d+%dms game as there is game in progress",
              command_parser.getTime(), command_parser.getIncrement());
          }
          break;
        case STOP_GAME:
          game.stop();
          video_capture.stop_game();
          break;
        case RESUME_GAME:
          video_capture.resume_game();
          game.resume([&]() {
            video_capture.stop_game();
          });
          break;
        case NO_COMMAND:
          break;
      }
    }
  );
  audio_capture.start([&](const float *samples, int count) {
    speech_to_text.add_audio(samples, count);
  });
  return 0;
}
