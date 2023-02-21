#include "sensorData.h"
#include <string.h>

void initSensorData(SensorData* sensorData, float* rotation_matrix, float* gyro, float height, float d_height){
	sensorData->rotation_matrix = rotation_matrix;
	sensorData->gyro = gyro;
	sensorData->height = height;
	sensorData->d_height = d_height;
}
