#ifndef CONTROLS_SENSOR_DATA
#define CONTROLS_SENSOR_DATA

struct _SensorData {
	float* rotation_matrix;
	float* gyro;
	float height;
	float d_height;
};
typedef struct _SensorData SensorData;

void initSensorData(SensorData* sensorData, float* rotation_matrix, float* gyro, float height, float d_height);

#endif
