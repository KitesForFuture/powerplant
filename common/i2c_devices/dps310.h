#ifndef I2C_DEVICES_BMP280
#define I2C_DEVICES_BMP280

#include "interchip.h"

struct _DPS310 {
	int dps_address;
	struct i2c_bus dps310_bus;
	Time last_update ;

	float current_height ;
	float height_medium_smooth ;

	float p_comp_init ;
	int p_init_counter ;

	uint8_t coefficientsource ;

	int c0, c1;

	int c00, c10, c01, c11, c20, c21, c30;

	uint8_t high_byte;
	uint8_t medium_byte;
	uint8_t low_byte;

	float p_raw;
	float t_raw;

	float lastHeight ;
	Time derivativeTimeStep ;
	float dH ;
};
typedef struct _DPS310 DPS310;

int update_dps310_if_necessary(DPS310 *dps);

void init_dps310(DPS310 *dps, struct i2c_bus bus_arg, int address_arg);

//float getPressureDiff();

float getHeight(DPS310 *dps);
float getHeightDerivative(DPS310 *dps);

#endif
