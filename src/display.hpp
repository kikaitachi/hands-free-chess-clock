#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <chrono>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <string>
#include <thread>

using namespace std::chrono_literals;

const int ALPHA_CMD_SYSTEM_SETUP = 0b00100000;
const int ALPHA_CMD_DIMMING_SETUP = 0b11100000;
const int ALPHA_CMD_DISPLAY_SETUP = 0b10000000;

const int ALPHA_BLINK_RATE_NOBLINK = 0b00;
const int ALPHA_DISPLAY_ON = 0b1;

const int FIRST_ADDRESS = 0x70;
const int DISPLAYS = 2;

class Display {
 public:
  Display();

  void set_white(std::string text);

  void set_black(std::string text);

 private:
  int fd;
};

#endif  // DISPLAY_H_
