#include "text_to_speech.hpp"

TextToSpeech::TextToSpeech(unsigned int sample_rate, std::string device)
    : audio_playback(sample_rate, device) {
}
