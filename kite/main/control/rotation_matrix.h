#ifndef ROTATION_MATRIX_FILE
#define ROTATION_MATRIX_FILE

#include <string.h>
#include "../../../common/helpers/math.h"
#include "../../../common/i2c_devices/mpu6050.h"
#include "../../../common/helpers/timer.h"


struct _Orientation_Data{
	float rotation_matrix[9];
	float rotation_matrix_transpose[9];
	float gyro_in_kite_coords[3];
};
typedef struct _Orientation_Data Orientation_Data;

//float rotation_matrix[9];
//float gyro_in_kite_coords[3];

float getAccelX();
float getAccelY();
float getAccelZ();

void initRotationMatrix(Orientation_Data* orientation_data);

void FAKEupdateRotationMatrix(Orientation_Data* orientation_data);
void updateRotationMatrix(Orientation_Data* orientation_data);


#endif
