#ifndef ROTATION_MATRIX_FILE
#define ROTATION_MATRIX_FILE

#include <string.h>
#include "../../../common/helpers/math.h"
#include "../../../common/i2c_devices/icm20948.h"
#include "../../../common/helpers/timer.h"


struct _Orientation_Data{
	float rotation_matrix[9];
	float rotation_matrix_transpose[9];
	float gyro_in_kite_coords[3];
	float line_vector_normed[3];
	Time mpu_last_update_time;
};
typedef struct _Orientation_Data Orientation_Data;

//float rotation_matrix[9];
//float gyro_in_kite_coords[3];

float getAccelX(raw_data_ICM20948 mpu_raw_data);
float getAccelY(raw_data_ICM20948 mpu_raw_data);
float getAccelZ(raw_data_ICM20948 mpu_raw_data);

void initRotationMatrix(Orientation_Data* orientation_data);

void FAKEupdateRotationMatrix(Orientation_Data* orientation_data);
void updateRotationMatrix(Orientation_Data* orientation_data, raw_data_ICM20948 mpu_raw_data);

void turnYAxisTowards(Orientation_Data* orientation_data, float x, float y);


#endif
