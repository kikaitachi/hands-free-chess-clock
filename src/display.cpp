#include "display.hpp"
#include "logger.hpp"
#include <cstring>

static const uint16_t segments[] {
	// nmlkjihgfedcba
	0b00000000000000, // ' ' (space)
	0b00001000001000, // '!'
	0b00001000000010, // '"'
 	0b01001101001110, // '#'
	0b01001101101101, // '$'
	0b10010000100100, // '%'
	0b00110011011001, // '&'
	0b00001000000000, // '''
	0b00000000111001, // '('
	0b00000000001111, // ')'
	0b11111010000000, // '*'
	0b01001101000000, // '+'
	0b10000000000000, // ','
	0b00000101000000, // '-'
	0b00000000000000, // '.'
	0b10010000000000, // '/'
	0b00000000111111, // '0'
	0b00010000000110, // '1'
	0b00000101011011, // '2'
	0b00000101001111, // '3'
	0b00000101100110, // '4'
	0b00000101101101, // '5'
	0b00000101111101, // '6'
	0b01010000000001, // '7'
	0b00000101111111, // '8'
	0b00000101100111, // '9'
	0b00000000000000, // ':'
	0b10001000000000, // ';'
	0b00110000000000, // '<'
	0b00000101001000, // '='
	0b01000010000000, // '>'
	0b01000100000011, // '?'
	0b00001100111011, // '@'
	0b00000101110111, // 'A'
	0b01001100001111, // 'B'
	0b00000000111001, // 'C'
	0b01001000001111, // 'D'
	0b00000101111001, // 'E'
	0b00000101110001, // 'F'
	0b00000100111101, // 'G'
	0b00000101110110, // 'H'
	0b01001000001001, // 'I'
	0b00000000011110, // 'J'
	0b00110001110000, // 'K'
	0b00000000111000, // 'L'
	0b00010010110110, // 'M'
	0b00100010110110, // 'N'
	0b00000000111111, // 'O'
	0b00000101110011, // 'P'
	0b00100000111111, // 'Q'
	0b00100101110011, // 'R'
	0b00000110001101, // 'S'
	0b01001000000001, // 'T'
	0b00000000111110, // 'U'
	0b10010000110000, // 'V'
	0b10100000110110, // 'W'
	0b10110010000000, // 'X'
	0b01010010000000, // 'Y'
	0b10010000001001, // 'Z'
	0b00000000111001, // '['
	0b00100010000000, // '\'
	0b00000000001111, // ']'
	0b10100000000000, // '^'
	0b00000000001000, // '_'
	0b00000010000000, // '`'
	0b00000101011111, // 'a'
	0b00100001111000, // 'b'
	0b00000101011000, // 'c'
	0b10000100001110, // 'd'
	0b00000001111001, // 'e'
	0b00000001110001, // 'f'
	0b00000110001111, // 'g'
	0b00000101110100, // 'h'
	0b01000000000000, // 'i'
	0b00000000001110, // 'j'
	0b01111000000000, // 'k'
	0b01001000000000, // 'l'
	0b01000101010100, // 'm'
	0b00100001010000, // 'n'
	0b00000101011100, // 'o'
	0b00010001110001, // 'p'
	0b00100101100011, // 'q'
	0b00000001010000, // 'r'
	0b00000110001101, // 's'
	0b00000001111000, // 't'
	0b00000000011100, // 'u'
	0b10000000010000, // 'v'
	0b10100000010100, // 'w'
	0b10110010000000, // 'x'
	0b00001100001110, // 'y'
	0b10010000001001, // 'z'
	0b10000011001001, // '{'
	0b01001000000000, // '|'
	0b00110100001001, // '}'
	0b00000101010010  // '~'
};

Display::Display() {
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

  uint8_t command = ALPHA_CMD_SYSTEM_SETUP | 1; // Enable system clock
  for (int i = 0; i < DISPLAYS; i++) {
    msg[i].addr = FIRST_ADDRESS + i;
    msg[i].flags = 0;
    msg[i].len = 1;
    msg[i].buf = &command;
  }
  if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
    logger::last("Failed to enable display system clock");
  }
  std::this_thread::sleep_for(1ms);

  command = ALPHA_CMD_DIMMING_SETUP | 15;
  if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
    logger::last("Failed to setup dimming");
  }

  command = ALPHA_CMD_DISPLAY_SETUP | (ALPHA_BLINK_RATE_NOBLINK << 1) | ALPHA_DISPLAY_ON;
  if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
    logger::last("Failed to setup display");
  }

  set_white("");
  set_black("");
}

void Display::set_white(std::string text) {
  logger::debug("Updating white display");
  illuminate_text(text, 0);
}

void Display::set_black(std::string text) {
  logger::debug("Updating black display");
  illuminate_text(text, 1);
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

	mem[adr + 1] = mem[adr + 1] | dat;
}

void Display::illuminate_segments(uint16_t segments, uint8_t digit) {
  for (uint8_t i = 0; i < 14; i++) { // There are 14 segments on this display
		if ((segments >> i) & 0b1) {
			illuminate_segment('A' + i, digit); // Convert the segment number to a letter
    }
	}
}

void Display::illuminate_char(char c, uint8_t digit) {
  illuminate_segments(segments[c - ' '], digit);
}

void Display::illuminate_text(std::string text, int black) {
  memset(&mem[16 * black], 0, 16);
  for (int i = 0, digit = 0; i < text.size(); i++) {
    if (text[i] == ':') {
      mem[16 * black + 2] |= 1;
    } else if (text[i] == '.') {
      mem[16 * black + 4] |= 1;
    } else {
      illuminate_char(text[i], digit + 4 * black);
      digit++;
      if (digit == 4) {
        break;
      }
    }
  }
  struct i2c_msg msg[2];
  struct i2c_rdwr_ioctl_data i2c_data;
  i2c_data.msgs = msg;
  i2c_data.nmsgs = 2;

  uint8_t dataToWrite = 0;
  msg[0].addr = FIRST_ADDRESS + black;
  msg[0].flags = 0;
  msg[0].len = 1;
  msg[0].buf = &dataToWrite;

  msg[1].addr = FIRST_ADDRESS + black;
  msg[1].flags = I2C_M_STOP;
  msg[1].len = 16;
  msg[1].buf = &mem[16 * black];
  if (ioctl(fd, I2C_RDWR, &i2c_data) < 0) {
    logger::last("Failed to update display");
  }
}
