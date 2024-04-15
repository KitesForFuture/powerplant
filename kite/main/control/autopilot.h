#ifndef CONTROLS_AUTOPILOT
#define CONTROLS_AUTOPILOT

#include "./controlData.h"
#include "./sensorData.h"
#include "./actuator.h"

#define HOVER_MODE 0
#define EIGHT_MODE 1
#define HOVER_EIGHT_TRANSITION 2
#define STRAIGHT_MODE 3
#define LANDING_MODE 4
#define LANDING_MODE_HOVER 5 // deprecated
#define LANDING_EIGHT_TRANSITION 6
#define FINAL_LANDING_MODE 7
#define FINAL_LANDING_MODE_HOVER 8 // deprecated
#define AIRPLANE_MODE 9
#define TEST_MODE 10
#define NOTLANDUNG 112

#define FIRST_TURN_MULTIPLIER 0.5

#define LEFT 0
#define RIGHT 1

#define LINE_TENSION_LAUNCH 0
#define LINE_TENSION_EIGHT -1
#define LINE_TENSION_LANDING -2
#define LINE_TENSION_LANDING_HOVER -3

struct _Autopilot {
	struct {
		struct {
			float P;
			float D;
		} Y;
		
		struct {
			float P;
			float D;
		} Z;
		
		struct {
			float P;
			float D;
		} X;
		
		struct {
			float P;
			float D;
		} H;
		float y_angle_offset;
	} hover;
	
	struct {
		struct {
			float P;
			float D;
		} Z;
		struct {
			float P;
			float D;
		} roll;
		struct {
			float P;
			float D;
		} Y;
		float elevator;
		float desired_line_angle_from_zenith;
		float target_angle_beta_clamp;
		float neutral_beta_sideways_flying_angle_fraction;
		float beta_P;
	} eight;
	
	struct {
		struct {
			float P;
			float D;
		} X;
		struct {
			float P;
			float D;
		} roll;
		struct {
			float P;
			float D;
		} Y;
		float desired_height;
		float dive_angle_P;
	} landing;
	
	float transition_y_angle_offset;
	
	float brake;
	
	float fm;
	
	float y_angle_offset;
	float desired_height;
		
	//struct {...} figure_eight;
		
	int mode;
	int direction;
	float sideways_flying_time;
		
	float multiplier;
	float turning_speed;
	Actuator slowly_changing_target_angle;
		
	float old_line_length;
	float smooth_reel_in_speed;
	
	float RC_target_angle;
	float RC_switch;
	
	Time timer;
};
typedef struct _Autopilot Autopilot;

float getLineRollAngle(float* line_dir);
float getLineYawAngle(float* line_dir);

void loadConfigVariables(Autopilot* autopilot, float* config_values);

void initAutopilot(Autopilot* autopilot, float* config_values);

void stepAutopilot(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension);

void notlandung(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data);
void test_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension);
void landing_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension, int transition);
void eight_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float timestep_in_s);
void straight_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length);
void hover_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension);

void normal_flight_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension, int transition);

#endif
