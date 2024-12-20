#ifndef I2C_DEVICES_BMP280P
#define I2C_DEVICES_BMP280P

#include "interchip.h"
#include "../helpers/timer.h"

struct dps310_struct {
    struct i2c_bus bus;
    int address;
    float current_height;
    float height_medium_smooth;
    
    float p_comp_init;
	int p_init_counter;
    
    int c0, c1;

	int c00, c10, c01, c11, c20, c21, c30;
	
	float lastHeight;
	Time derivativeTimeStep;
	float dH;
	float old_dH;
	
	float p_raw, t_raw;
};

int update_dps310_if_necessary_p(struct dps310_struct *dps);

void init_dps310_p(struct dps310_struct *dps);

//float getPressureDiff();

float getHeight_p(struct dps310_struct *dps);
float getHeightDerivative_p(struct dps310_struct *dps);

#endif
