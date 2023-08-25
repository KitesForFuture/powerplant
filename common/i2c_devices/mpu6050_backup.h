#ifndef I2C_DEVICES_MPU6050
#define I2C_DEVICES_MPU6050

#include "freertos/FreeRTOS.h"
#include "interchip.h"

struct _Mpu_raw_data {
	float accel[3];
	float gyro[3];
};
typedef struct _Mpu_raw_data Mpu_raw_data;

struct _MPU {
	Mpu_raw_data calibration_data;
	struct i2c_bus bus;
	int address;
	float gyro_precision_factor;	//factor needed to get to deg/sec
	float accel_precision_factor;	//factor needed to get to m/s
};
typedef struct _MPU MPU;

void initMPU6050(MPU *mpu);

void readMPUData(MPU *mpu, Mpu_raw_data *out);

#endif
