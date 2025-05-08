#ifndef CONTROLS_SENSOR_DATA
#define CONTROLS_SENSOR_DATA

struct _SensorData {
	float* rotation_matrix;
	float* line_direction_vector;
	float* gyro;
	float height;
	float d_height;
	float speed_pitot;
};
typedef struct _SensorData SensorData;

void initSensorData(SensorData* sensorData, float* rotation_matrix, float* line_direction_vector, float* gyro, float height, float d_height, float speed_pitot);

#endif
