#ifndef I2C_DEVICES_CAT24C256
#define I2C_DEVICES_CAT24C256

#include "interchip.h"

void init_cat24(struct i2c_bus bus_arg);

void write2EEPROM(float number, int address);

float readEEPROM(int address);

#endif