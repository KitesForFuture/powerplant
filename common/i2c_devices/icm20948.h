#ifndef I2C_DEVICES_ICM20948
#define I2C_DEVICES_ICM20948

#include "freertos/FreeRTOS.h"
#include "interchip.h"

struct _raw_data_ICM20948 {
	float accel[3];
	float gyro[3];
	float magnet[3];
};
typedef struct _raw_data_ICM20948 raw_data_ICM20948;

struct _ICM20948 {
	raw_data_ICM20948 calibration_data;
	struct i2c_bus bus;
	int address;
	int magnetometer_address;
	float gyro_precision_factor;	//factor needed to get to deg/sec
	float accel_precision_factor;	//factor needed to get to m/s
	float magnet_precision_factor;	//factor needed to get to m/s
};
typedef struct _ICM20948 ICM20948;

void initICM20948(ICM20948 *mpu);

void readRawDataICM20948(ICM20948 *mpu, raw_data_ICM20948 *out);
void readDataICM20948(ICM20948 *mpu, raw_data_ICM20948 *out);

#endif
