#include "sensorData.h"
#include <string.h>

void initSensorData(SensorData* sensorData, float* rotation_matrix, float* line_direction_vector, float* gyro, float height, float d_height, float speed_pitot){
	sensorData->rotation_matrix = rotation_matrix;
	sensorData->line_direction_vector = line_direction_vector;
	sensorData->gyro = gyro;
	sensorData->height = height;
	sensorData->d_height = d_height;
	sensorData->speed_pitot = speed_pitot;
}
