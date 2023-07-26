#ifndef ROTATION_MATRIX_FILE
#define ROTATION_MATRIX_FILE

#include <string.h>
#include "../../../common/helpers/math.h"
#include "../../../common/i2c_devices/mpu6050.h"
#include "../../../common/i2c_devices/mpu9250.h"
#include "../../../common/helpers/timer.h"


struct _Orientation_Data{
	float rotation_matrix[9];
	float rotation_matrix_transpose[9];
	float gyro_in_kite_coords[3];
	Time mpu_last_update_time;
};
typedef struct _Orientation_Data Orientation_Data;

//float rotation_matrix[9];
//float gyro_in_kite_coords[3];

float getAccelX(Mpu_raw_data_9250 mpu_raw_data);
float getAccelY(Mpu_raw_data_9250 mpu_raw_data);
float getAccelZ(Mpu_raw_data_9250 mpu_raw_data);

void initRotationMatrix(Orientation_Data* orientation_data);
void initLineMatrix(Orientation_Data* orientation_data);

void FAKEupdateRotationMatrix(Orientation_Data* orientation_data);
void updateRotationMatrix(Orientation_Data* orientation_data, Mpu_raw_data_9250 mpu_raw_data);

void turnYAxisTowards(Orientation_Data* orientation_data, float x, float y);


#endif
