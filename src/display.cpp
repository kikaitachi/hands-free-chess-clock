#include "display.hpp"
#include "logger.hpp"
#include <cstring>

Display::Display() {
  memset(mem, 0, sizeof(mem));
  std::string i2c_bus = "/dev/i2c-7";
  if ((fd = open(i2c_bus.c_str(), O_RDWR)) == -1) {
    logger::last("Failed to open i2c bus %s", i2c_bus.c_str());
  } else {
    logger::info("Opened i2c bus %s", i2c_bus.c_str());
  }

  struct i2c_msg msg[DISPLAYS];
  struct i2c_rdwr_ioctl_data i2c_data;
  i2c_data.msgs = msg;
  i2c_data.nmsgs = DISPLAYS;

  uint8_t dataToWrite = ALPHA_CMD_SYSTEM_SETUP | 1; // Enable system clock
  for (int i = 0; i < DISPLAYS; i++) {
    msg[i].addr = FIRST_ADDRESS + i;
    msg[i].flags = 0;
    msg[i].len = 1;
    msg[i].buf = &dataToWrite;
  }
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

void Display::set_white(std::string text) {
  logger::debug("Updating white display");
  /*illuminate_segment('A', 1);
  illuminate_segment('B', 1);
  illuminate_segment('C', 1);
  illuminate_segment('D', 1);
  illuminate_segment('E', 1);
  illuminate_segment('F', 1);
  illuminate_segment('G', 1);
  illuminate_segment('H', 1);
  illuminate_segment('I', 1);
  illuminate_segment('J', 1);
  illuminate_segment('K', 1);
  illuminate_segment('L', 1);
  illuminate_segment('M', 1);
  illuminate_segment('N', 1);

  illuminate_segment('A', 3);
  illuminate_segment('B', 3);
  illuminate_segment('C', 3);
  illuminate_segment('D', 3);
  illuminate_segment('E', 3);
  illuminate_segment('F', 3);
  illuminate_segment('G', 3);
  illuminate_segment('H', 3);
  illuminate_segment('I', 3);
  illuminate_segment('J', 3);
  illuminate_segment('K', 3);
  illuminate_segment('L', 3);
  illuminate_segment('M', 3);
  illuminate_segment('N', 3);*/
  illuminate_char(0b00000101110111, 0);
  illuminate_char(0b00000000111111, 1);
  illuminate_char(0b00000101001111, 2);
  illuminate_char(0b00000101100111, 3);
  //mem[2] = 1;
  //mem[4] = 1;
  //mem[4] = 1;
  //mem[15] = 0xFF;
  //mem[5] = 0b11000000;
  struct i2c_msg msg[2];
  struct i2c_rdwr_ioctl_data i2c_data;
  i2c_data.msgs = msg;
  i2c_data.nmsgs = 2;

  uint8_t dataToWrite = 0;
  msg[0].addr = FIRST_ADDRESS;
  msg[0].flags = 0;
  msg[0].len = 1;
  msg[0].buf = &dataToWrite;

  msg[1].addr = FIRST_ADDRESS;
  msg[1].flags = I2C_M_STOP;
  msg[1].len = 16;
  msg[1].buf = mem;
  if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
    logger::last("Failed to update display");
  }
}

void Display::set_black(std::string text) {
  logger::debug("Updating black display");
  /*illuminate_segment('A', 5);
  illuminate_segment('B', 5);
  illuminate_segment('C', 5);
  illuminate_segment('D', 5);
  illuminate_segment('E', 5);
  illuminate_segment('F', 5);
  illuminate_segment('G', 5);
  illuminate_segment('H', 5);
  illuminate_segment('I', 5);
  illuminate_segment('J', 5);
  illuminate_segment('K', 5);
  illuminate_segment('L', 5);
  illuminate_segment('M', 5);
  illuminate_segment('N', 5);*/
  /*mem[18] = 1;
  mem[19] = 1;
  mem[20] = 1;*/
  struct i2c_msg msg[2];
  struct i2c_rdwr_ioctl_data i2c_data;
  i2c_data.msgs = msg;
  i2c_data.nmsgs = 2;

  uint8_t dataToWrite = 0;
  msg[0].addr = FIRST_ADDRESS + 1;
  msg[0].flags = 0;
  msg[0].len = 1;
  msg[0].buf = &dataToWrite;

  /*uint8_t data[16] = {
    0b00000000,
    0b00000000,
    0b00000001,
    0b00000001,
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
    0b00000000
  };*/
  msg[1].addr = FIRST_ADDRESS + 1;
  msg[1].flags = I2C_M_STOP;
  msg[1].len = 16;
  msg[1].buf = &mem[16];
  if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
    logger::last("Failed to update display");
  }
}

void Display::illuminate_segment(uint8_t segment, uint8_t digit) {
  uint8_t com;
	uint8_t row;

	com = segment - 'A'; // Convert the segment letter back to a number

	if (com > 6)
		com -= 7;
	if (segment == 'I')
		com = 0;
	if (segment == 'H')
		com = 1;

	row = digit % 4; // Convert digit (1 to 16) back to a relative position on a given digit on display
	if (segment > 'G')
		row += 4;

	uint8_t offset = digit / 4 * 16;
	uint8_t adr = com * 2 + offset;

	// Determine the address
	if (row > 7)
		adr++;

	// Determine the data bit
	if (row > 7)
		row -= 8;
	uint8_t dat = 1 << row;

  //logger::debug("Segment %c, adr: %d, dat: %d", segment, (int)adr, (int)dat);

	mem[adr + 1] = mem[adr + 1] | dat;
}

void Display::illuminate_char(uint16_t segmentsToTurnOn, uint8_t digit) {
  for (uint8_t i = 0; i < 14; i++) { // There are 14 segments on this display
		if ((segmentsToTurnOn >> i) & 0b1) {
			illuminate_segment('A' + i, digit); // Convert the segment number to a letter
    }
	}
}
