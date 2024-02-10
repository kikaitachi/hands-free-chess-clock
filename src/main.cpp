#include "audio_capture.hpp"
#include "command_parser.hpp"
#include "game.hpp"
#include "logger.hpp"
#include "speech_to_text.hpp"

#include <string>

int main() {
  Game game;
  CommandParser command_parser;
  SpeechToText speech_to_text;
  AudioCapture audio_capture(speech_to_text.getSampleRate(), "plughw:DEV=0,CARD=C920");
  speech_to_text.start(
    [&]() {
      game.ready();
    },
    [&](const std::string speech) {
      Command command = command_parser.recognised(speech);
      if (command != NO_COMMAND) {
        // Ensure the same command will not be 'heard' multiple times
        speech_to_text.clear_audio();
      }
      switch (command) {
        case START_GAME:
          if (!game.playing) {
            game.start(command_parser.getTime(), command_parser.getIncrement());
          } else {
            logger::info("Not starting new %d+%dms game as there is game in progress",
              command_parser.getTime(), command_parser.getIncrement());
          }
          break;
        case STOP_GAME:
          game.stop();
          break;
        case RESUME_GAME:
          game.resume();
          break;
        case SWITCH_CLOCK:
          game.switch_clock();
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
