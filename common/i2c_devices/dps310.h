#ifndef I2C_DEVICES_BMP280
#define I2C_DEVICES_BMP280

#include "interchip.h"

int update_dps310_if_necessary();

void init_dps310(struct i2c_bus bus_arg);

//float getPressureDiff();

float getHeight();
float getHeightDerivative();

#endif
