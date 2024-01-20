module;

import logger;

#include <chrono>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <string>
#include <thread>

export module display;

using namespace std::chrono_literals;

const int ALPHA_CMD_SYSTEM_SETUP = 0b00100000;
const int ALPHA_CMD_DIMMING_SETUP = 0b11100000;
const int ALPHA_CMD_DISPLAY_SETUP = 0b10000000;

const int ALPHA_BLINK_RATE_NOBLINK = 0b00;
const int ALPHA_DISPLAY_ON = 0b1;

export class Display {
 public:
  Display() {
    std::string i2c_bus = "/dev/i2c-7";
    if ((fd = open(i2c_bus.c_str(), O_RDWR)) == -1) {
      logger::last("Failed to open i2c bus %s", i2c_bus.c_str());
    } else {
      logger::info("Opened i2c bus %s", i2c_bus.c_str());
    }

    struct i2c_msg msg[1];
    struct i2c_rdwr_ioctl_data i2c_data;
    i2c_data.msgs = msg;
    i2c_data.nmsgs = 1;

    uint8_t dataToWrite = ALPHA_CMD_SYSTEM_SETUP | 1; // Enable system clock
    msg[0].addr = 0x70;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &dataToWrite;
    if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
      logger::last("Failed to enable display system clock");
    }
    std::this_thread::sleep_for(1ms);

    dataToWrite = ALPHA_CMD_DIMMING_SETUP | 15;
    if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
      logger::last("Failed to setup dimming");
    }

    dataToWrite = ALPHA_CMD_DISPLAY_SETUP | (ALPHA_BLINK_RATE_NOBLINK << 1) | ALPHA_DISPLAY_ON;
    if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
      logger::last("Failed to setup display");
    }
  }

  void set_white(std::string text) {
    logger::debug("Udating display");
    struct i2c_msg msg[2];
    struct i2c_rdwr_ioctl_data i2c_data;
    i2c_data.msgs = msg;
    i2c_data.nmsgs = 2;

    uint8_t dataToWrite = 0;
    msg[0].addr = 0x70;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &dataToWrite;

    uint8_t data[16] = {
      0b00000000,
      0b00000000,
      0b00000001,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000
    };
    msg[1].addr = 0x70;
    msg[1].flags = I2C_M_STOP;
    msg[1].len = 16;
    msg[1].buf = data;
    if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
      logger::last("Failed to update display");
    }
  }

 private:
  int fd;
};