#include "audio_capture.hpp"
#include "command_parser.hpp"
#include "game.hpp"
#include "logger.hpp"
#include "openings.hpp"
#include "process.hpp"
#include "speech_to_text.hpp"

#include <cstdlib>
#include <string>

int main(int argc, char** argv) {
  std::string audio_input = "default";
  if (argc > 1) {
    audio_input = std::string(argv[1]);
  }
  std::string audio_output = "default";
  if (argc > 2) {
    audio_output = std::string(argv[2]);
  }
  std::string uci_engine;
  if (argc > 3) {
    uci_engine = argv[3];
  } else {
    uci_engine = "/usr/games/stockfish";
  }

  char const *piper_argv[] = {
    "piper/piper", "--model", "models/en_US-amy-medium.onnx", "-q", "--output_raw", nullptr
  };
  Process piper_process(piper_argv);

  chess::Openings openings("openings");

  Game game(openings, audio_output, uci_engine, piper_process);
  CommandParser command_parser;
  SpeechToText speech_to_text;
  AudioCapture audio_capture(speech_to_text.getSampleRate(), audio_input);
  speech_to_text.start(
    [&]() {
      game.ready();
    },
    [&](const std::string speech) {
      Command command = command_parser.recognised(speech);
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
        case SHUTDOWN:
          game.shutdown();
          if (std::system("sudo shutdown now") == -1) {
            logger::last("Failed to shutdown");
          }
          break;
        case BEST_MOVE:
          game.best_move();
          break;
        case WHO_IS_WINNING:
          game.who_is_winning();
          break;
        case NO_COMMAND:
          break;
      }
    }
  );
  audio_capture.start([&](std::vector<float>& audio, int start, int end) {
    speech_to_text.add_audio(audio, start, end);
  });
  return 0;
}
