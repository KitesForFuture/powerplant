#include "sensorData.h"
#include <string.h>

void initSensorData(SensorData* sensorData, float* rotation_matrix, float* rotation_matrix_line, float* gyro, float height, float d_height){
	sensorData->rotation_matrix = rotation_matrix;
	sensorData->rotation_matrix_line = rotation_matrix_line;
	sensorData->gyro = gyro;
	sensorData->height = height;
	sensorData->d_height = d_height;
}
