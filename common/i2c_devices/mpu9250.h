#ifndef I2C_DEVICES_MPU9250
#define I2C_DEVICES_MPU9250

#include "freertos/FreeRTOS.h"
#include "interchip.h"

struct _Mpu_raw_data_9250 {
	float accel[3];
	float gyro[3];
	float magnet[3];
};
typedef struct _Mpu_raw_data_9250 Mpu_raw_data_9250;

struct _MPU9250 {
	Mpu_raw_data_9250 calibration_data;
	struct i2c_bus bus;
	int address;
	int magnetometer_address;
	float gyro_precision_factor;	//factor needed to get to deg/sec
	float accel_precision_factor;	//factor needed to get to m/s
	float magnet_precision_factor;	//factor needed to get to m/s
};
typedef struct _MPU9250 MPU9250;

void initMPU9250(MPU9250 *mpu);

void readMPUData9250(MPU9250 *mpu, Mpu_raw_data_9250 *out);

#endif
