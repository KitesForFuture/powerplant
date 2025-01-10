#include "../../../common/helpers/math.h"
#include "../../../common/helpers/timer.h"
#include "autopilot.h"

#define DATA_MODE 1

#define LINE_TENSION_REQUEST_MODE 2

#define AIRBRAKE_ON 90
#define AIRBRAKE_OFF -90

float old_line_length = 0;
float powerInWatts = 0;

void sendDebuggingData(float num1, float num2, float num3, float num4, float num5, float num6);
void sendData(uint32_t mode, float data0, float data1, float data2);

float angleDifferenceWithoutDiscontinuity(float alpha, float beta){
	float diff = alpha - beta;
	if(diff > 3.141592){
		diff -= 2*3.141592;
	}else if(diff < -3.141592){
		diff += 2*3.141592;
	}
	return diff;
	/* //fmod way by Grok
	float diff = alpha - beta;
    diff = fmod(diff + M_PI, 2 * M_PI) - M_PI; // fmod for floating-point modulo
    return diff;
	*/
}

// TODO. calculate the correct error even when offset is 180 degrees.
float getAngleError(float offset, float controllable_axis[3], float axis_we_wish_horizontal[3]){
	
	// cross product with (1,0,0)
	float where_axis_we_wish_horizontal_should_be[3];
	where_axis_we_wish_horizontal_should_be[0] = 0;
	where_axis_we_wish_horizontal_should_be[1] = controllable_axis[2];
	where_axis_we_wish_horizontal_should_be[2] = -controllable_axis[1];
	
	float l_1_norm = fabs(where_axis_we_wish_horizontal_should_be[1]) + fabs(where_axis_we_wish_horizontal_should_be[2]);
	
	if(l_1_norm < 0.05){ // close to undefinedness
		return 0;
	}else{
		normalize(where_axis_we_wish_horizontal_should_be, 3);
		return sign(axis_we_wish_horizontal[0]) * angle(axis_we_wish_horizontal, where_axis_we_wish_horizontal_should_be) - offset;
	}
}

float getAngleErrorRollInHorizontalFlight(float offset, float mat[9]){
	float controllable_axis[3] = {-mat[0], -mat[1], -mat[2]};
	float axis_we_wish_horizontal[3] = {mat[3], mat[4], mat[5]};
	return -getAngleError(offset, controllable_axis, axis_we_wish_horizontal);
}

float getAngleErrorZAxis(float offset, float mat[9]){
	float controllable_axis[3] = {mat[6], mat[7], mat[8]};
	float axis_we_wish_horizontal[3] = {mat[3], mat[4], mat[5]};
	//printf("OLD: contr_axis = %f, %f, %f, awh = %f, %f, %f\n", controllable_axis[0], controllable_axis[1], controllable_axis[2], axis_we_wish_horizontal[0], axis_we_wish_horizontal[1], axis_we_wish_horizontal[2]);
	return getAngleError(offset, controllable_axis, axis_we_wish_horizontal);
}

float getAngleErrorZAxisImproved(float offset, float mat[9], float line_dir[3]){
	// the controllable axis is not the z-axis, but the line direction in world coordinates.
	float controllable_axis[3];
	mat_transp_mult_vec(mat, -line_dir[0], -line_dir[1], -line_dir[2], controllable_axis);
	
	// The y-axis can lie horizontal as a result of yaw and roll. Thus this cannot be used as axis that needs to be made horizontal.
	// Instead we use the vector orthogonal to nose-direction and line direction.
	float axis_we_wish_horizontal[3];
	crossProduct(controllable_axis[0], controllable_axis[1], controllable_axis[2], mat[0], mat[1], mat[2], axis_we_wish_horizontal);
	
	//printf("NEW: line = %f, %f, %f, contr_axis = %f, %f, %f, awh = %f, %f, %f, ", line_dir[0], line_dir[1], line_dir[2], controllable_axis[0], controllable_axis[1], controllable_axis[2], axis_we_wish_horizontal[0], axis_we_wish_horizontal[1], axis_we_wish_horizontal[2]);
	
	return getAngleError(offset, controllable_axis, axis_we_wish_horizontal);
}

float getAngleErrorYAxis(float offset, float mat[9]){
	float controllable_axis[3] = {mat[3], mat[4], mat[5]};
	float axis_we_wish_horizontal[3] = {-mat[6], -mat[7], -mat[8]};
	return getAngleError(offset, controllable_axis, axis_we_wish_horizontal);
}

float getLineAngle(float direction_of_neutral_angle, float direction_of_90_deg_angle){
	float angle = 0;//signed_angle2(flight_direction, line_direction);//angle(new THREE.Vector3(mat[4], mat[5], 0), new THREE.Vector3(mat_line[7], mat_line[8], 0)) - Math.PI*0.5;
	if(direction_of_neutral_angle == 0){
		angle = PI/2*sign(direction_of_90_deg_angle);
	}else if(direction_of_neutral_angle > 0){
		angle = atan(direction_of_90_deg_angle/direction_of_neutral_angle);
	}else if(direction_of_90_deg_angle < 0){
		angle = -PI+atan(direction_of_90_deg_angle/direction_of_neutral_angle);
	}else{
		angle = PI+atan(direction_of_90_deg_angle/direction_of_neutral_angle);
	}
	return angle;
}

float getLineRollAngle(float* line_dir){
	return 0.75*getLineAngle(-line_dir[2], line_dir[1]);
}

float getLineYawAngle(float* line_dir){
	return getLineAngle(line_dir[0], line_dir[1]);
}

float getLineYawAngleImprovedForLanding(float* line_dir, float* mat){
	
	// projecting kite flight direction to horizontal y,z-plane
	float nose_dir_y = mat[1];
	float nose_dir_z = mat[2];
	
	// transforming line direction back to global coordinates mat^(-1)*line_dir, then projecting to the horizontal y,z-plane
	float line_dir_y_global_coords = mat[1] * line_dir[0] + mat[4] * line_dir[1] + mat[7] * line_dir[2];
	float line_dir_z_global_coords = mat[2] * line_dir[0] + mat[5] * line_dir[1] + mat[8] * line_dir[2];
	
	// dividing one by the other (complex arithmetic) ignoring the correct length of the result
	float x = nose_dir_y * line_dir_y_global_coords + nose_dir_z * line_dir_z_global_coords;
	float y = nose_dir_y * line_dir_z_global_coords - nose_dir_z * line_dir_y_global_coords;
	
	// calculate signed angle
	return getLineAngle(x, y);
}

float getLinePitchAngle(float* line_dir){
	return 0.75*getLineAngle(-line_dir[2], -line_dir[0]);
}

void loadConfigVariables(Autopilot* autopilot, float* config_values){
	autopilot->hover.Y.P = config_values[14];
	autopilot->hover.Y.D = config_values[15];
	
	autopilot->hover.Z.P = config_values[17];
	autopilot->hover.Z.D = config_values[18];
	
	autopilot->hover.X.P = config_values[47];
	autopilot->hover.X.D = config_values[19];
	
	autopilot->hover.H.P = config_values[20];
	autopilot->hover.H.D = config_values[21];
	autopilot->hover.y_angle_offset = config_values[16]*PI/180;
	autopilot->transition_y_angle_offset = config_values[31]*PI/180;
	autopilot->eight.Y.D = config_values[29];
	
	autopilot->eight.Z.P = config_values[27];
	autopilot->eight.Z.D = config_values[28];
	
	autopilot->eight.roll.P = config_values[42];
	autopilot->eight.roll.D = config_values[43];
	
	autopilot->landing.X.P = config_values[25];
	autopilot->landing.X.D = config_values[44];
	autopilot->landing.roll.P = config_values[45];
	autopilot->landing.roll.D = config_values[46];
	autopilot->landing.Y.P = config_values[23];
	autopilot->landing.Y.D = config_values[24];
	autopilot->landing.desired_height = config_values[26];
	
	autopilot->brake = config_values[22];
	autopilot->sideways_flying_time = config_values[12];
	autopilot->turning_speed = config_values[13]*PI/180;
	
	
	autopilot->eight.elevator = config_values[30];
	autopilot->eight.desired_line_angle_from_zenith = PI/2-config_values[32]*PI/180;
	autopilot->eight.target_angle_beta_clamp = config_values[33];
	autopilot->eight.beta_P = config_values[34];
	autopilot->eight.neutral_beta_sideways_flying_angle_fraction = config_values[35];
	autopilot->landing.dive_angle_P = config_values[36];
}

void initAutopilot(Autopilot* autopilot, float* config_values){
    autopilot->fm = 0.0;
    
	loadConfigVariables(autopilot, config_values);
	
	autopilot->y_angle_offset = autopilot->hover.y_angle_offset;
	
	autopilot->mode = HOVER_MODE;
	autopilot->direction = 1;
	autopilot->multiplier = FIRST_TURN_MULTIPLIER;
	
	initActuator(&(autopilot->slowly_changing_target_angle), autopilot->turning_speed, -3.14, 3.14);
	autopilot->slowly_changing_target_angle.currentValue = 0.75;
	autopilot->timer = 0;

	autopilot->RC_target_angle = 0;
	autopilot->RC_switch = 0;
}

static int landing_propeller_counter = -1;

void stepAutopilot(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_speed, float line_tension){
	//printf("ap-mode = %d\n", autopilot->mode);
	/*if(autopilot->fm == 3.0){ // 3.0 is VESC final landing mode
		autopilot->mode = LANDING_MODE;
	}*/
	if (sensor_data.height > 10 && sensor_data.height > 1.5*line_length){
		autopilot->mode = NOTLANDUNG;
	}
	if(autopilot->mode == NOTLANDUNG){
		notlandung(autopilot, control_data_out, sensor_data); return;
	}
	//autopilot->mode = LANDING_MODE; // FOR DEBUGGING ONLY
	
	float timestep_in_s = 0.02; // 50 Hz, but TODO: MUST measure more precisely!
	if(autopilot->mode == AIRPLANE_MODE){
		landing_control(autopilot, control_data_out, sensor_data, line_length, line_speed, line_tension, false); return;
	}
	//autopilot->mode = EIGHT_MODE;
	//printf("mode = %d, fm = %f, line_length = %f\n", autopilot->mode, autopilot->fm, line_length);
	if(autopilot->mode == HOVER_MODE){
		if(autopilot->fm != 0.0 && autopilot->fm != 3.0){ // 0.0 is VESC launch mode, 3.0 is VESC final landing mode
			autopilot->mode = HOVER_EIGHT_TRANSITION;
			autopilot->timer = start_timer();
		}
		if(autopilot->fm == 3.0){ // 0.0 is VESC launch mode, 3.0 is VESC final landing mode
			autopilot->mode = LANDING_MODE;
			landing_propeller_counter = 0;
			old_line_length = line_length;
		}
		autopilot->y_angle_offset = autopilot->hover.y_angle_offset;
		
		hover_control(autopilot, control_data_out, sensor_data, line_length, LINE_TENSION_LAUNCH); return;
	}else if(autopilot->mode == HOVER_EIGHT_TRANSITION){
		if(query_timer_seconds(autopilot->timer) > 1){
			if(autopilot->fm == 3.0){
				autopilot->mode = LANDING_MODE;
			old_line_length = line_length;
			} else {
				autopilot->timer = start_timer();
				autopilot->multiplier = FIRST_TURN_MULTIPLIER;
				autopilot->mode = EIGHT_MODE;
				old_line_length = line_length;
				autopilot->y_angle_offset = autopilot->hover.y_angle_offset;
			}
		}
		autopilot->y_angle_offset = autopilot->transition_y_angle_offset;
		hover_control(autopilot, control_data_out, sensor_data, line_length, LINE_TENSION_EIGHT); return;
	}else if(autopilot->mode == EIGHT_MODE){
		if(autopilot->fm >= 2.0 && fabs(getAngleErrorZAxisImproved(0.0, sensor_data.rotation_matrix, sensor_data.line_direction_vector)) < 0.2){//landing mode request from VESC
			autopilot->mode = LANDING_MODE;
			old_line_length = line_length;
		}
		eight_control(autopilot, control_data_out, sensor_data, line_length, timestep_in_s); return;
	}else if(autopilot->mode == LANDING_MODE){
		//sendData(LINE_TENSION_REQUEST_MODE, 0.0, 0.0, 0.0);
		if(autopilot->fm == 1.0){ // 1.0 is VESC eight mode
			autopilot->mode = LANDING_EIGHT_TRANSITION;
		}
		landing_control(autopilot, control_data_out, sensor_data, line_length, line_speed, line_tension, false); return;
	}else if(autopilot->mode == LANDING_EIGHT_TRANSITION){
		if(sensor_data.rotation_matrix[0] > 0.1){
			autopilot->timer = start_timer();
			//autopilot->direction = 1;
			autopilot->multiplier = FIRST_TURN_MULTIPLIER;
			autopilot->mode = EIGHT_MODE;
			old_line_length = line_length;
		}
		return landing_control(autopilot, control_data_out, sensor_data, line_length, line_speed, line_tension, true);
	}else if(autopilot->mode == TEST_MODE){
		return test_control(autopilot, control_data_out, sensor_data, line_length, line_tension);
	}
}

static float desired_dive_angle_smooth = 0;
static float old_prop_speed = 0;

void test_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension){
	float* mat = sensor_data.rotation_matrix;
	
	float roll_angle = safe_asin(-mat[3]);
	
	// 2. STAGE P(I)D: flight direction -> neccessary roll angle
	float test_angle = autopilot->landing.desired_height;
	float flight_direction[2] = {mat[1], mat[2]};
	float desired_flight_direction[2] = {sin(test_angle), cos(test_angle)};
	float z_axis_offset = signed_angle2(flight_direction, desired_flight_direction);
	float desired_roll_angle = clamp(autopilot->eight.roll.P * z_axis_offset - autopilot->eight.roll.D * sensor_data.gyro[2], -60*PI/180, 60*PI/180);
	
	//desired_roll_angle = 0;
	
	// 3. STAGE
	float z_axis_control = - 0.56 * autopilot->eight.Z.P * (desired_roll_angle-roll_angle) - 0.22 * autopilot->eight.Z.D * sensor_data.gyro[0];
	z_axis_control *= 100;
	z_axis_control = clamp(0.5*z_axis_control, -30, 30);
	
	float y_axis_control = autopilot->eight.elevator + autopilot->eight.Y.D * sensor_data.gyro[1];
	//sendDebuggingData(3, z_axis_control, autopilot->eight.Z.P, autopilot->eight.Z.D, autopilot->eight.roll.P, autopilot->eight.roll.D);
	initControlData(control_data_out, 0, 0, y_axis_control - z_axis_control + abs(z_axis_control)*0.5, y_axis_control + z_axis_control + abs(z_axis_control)*0.5, 0, 0, 0, 0, LINE_TENSION_EIGHT); return;
}

void notlandung(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data){
	float* mat = sensor_data.rotation_matrix;
	
	// SLIGHT DIVE ANGLE
	float desired_dive_angle = 0.1;
	
	float y_axis_offset = -getAngleErrorYAxis(-desired_dive_angle - PI/2, mat);
	float y_axis_control = 45.0 * autopilot->landing.Y.P * y_axis_offset + 2.31 * autopilot->landing.Y.D * sensor_data.gyro[1];
	
	// FIXED ROLL ANGLE OF 30 DEGREES
	float desired_roll_angle = 30*PI/180;
	
	float roll_angle = getAngleErrorRollInHorizontalFlight(0.0, mat); // good in horizontal flight, but better at higher climb/sink rates
	
	float x_axis_control = - 28 * autopilot->landing.X.P * (desired_roll_angle-roll_angle) - 11 * autopilot->landing.X.D * sensor_data.gyro[0];
	x_axis_control = clamp(x_axis_control, -30, 30);
	y_axis_control = clamp(y_axis_control, -50, 50);
	
	sendDebuggingData(y_axis_offset, y_axis_control, sensor_data.height, 0, 0, 2); // UP-DOWN control
	
	// AIRBRAKE EXTENDED
	float airbrake = AIRBRAKE_ON;
	initControlData(control_data_out, 0, 0,
		autopilot->brake*(90+airbrake)*0.005 + y_axis_control - x_axis_control + abs(x_axis_control)*0.1,
		autopilot->brake*(90+airbrake)*0.005 + y_axis_control + x_axis_control + abs(x_axis_control)*0.1,
		0,
		0,
		airbrake, 0, LINE_TENSION_LANDING); return;
}

static float line_length_derivative = 0;
//static float line_length_derivative_counter = 0;
static float airbrake = -60;
static float airbrake_compensation_by_elevons = 0;

static int sendCounter = 0;
static int frequencyDivider = 2;

static float length2Height(float length){
	if(length < 5){
		return 0;
	}else if(length < 10){
		return (length - 5) * (length - 5) * 0.02;
	}else{
		return (length - 10)*0.2 + 0.5;
	}
}

void landing_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_speed, float line_tension, int transition){
	
	float* mat = sensor_data.rotation_matrix;
	float* line_dir = sensor_data.line_direction_vector;
	//printf("roll = %f, pitch = %f\n", getLineRollAngle(line_dir), getLinePitchAngle(line_dir));
	
	// HEIGHT CONTROL: STAGE 1
	float height = sensor_data.height-autopilot->landing.desired_height;
	
	//float height_error = height;//
	float height_error = height - length2Height(line_length);
	
	float desired_dive_angle = 0.15*autopilot->landing.dive_angle_P*height_error;//-desired_line_angle - 2.0 * line_angle_error;
	desired_dive_angle_smooth = desired_dive_angle;//0.8 * desired_dive_angle_smooth + 0.2 * desired_dive_angle;
	desired_dive_angle_smooth = clamp( desired_dive_angle_smooth, -PI/12, PI/6 );
	
	if(transition){
		desired_dive_angle_smooth = -PI/6;
	}
	//desired_dive_angle_smooth = 0.0;
	// HEIGHT CONTROL: STAGE 2
	float y_axis_offset = -getAngleErrorYAxis(-desired_dive_angle_smooth - PI/2, mat);
	float y_axis_control = 45.0 * autopilot->landing.Y.P * y_axis_offset + 2.31 * autopilot->landing.Y.D * sensor_data.gyro[1];
	
	
	// LEFT-RIGHT CONTROL
	//float angle_error_old = getLineYawAngle(line_dir);
	float angle_error = getLineYawAngleImprovedForLanding(line_dir, mat);
	//printf("old angle = %f, new angle = %f\n", angle_error_old, angle_error_old);
	
	//for TESTING HAND LAUNCH:
	/*if(mat[0] > 0.12){
		angle_error = getAngleErrorZAxisImproved(0.0, mat, line_dir);
	}*/
	
	float desired_roll_angle = clamp(autopilot->landing.roll.P * angle_error - autopilot->landing.roll.D * sensor_data.gyro[2], -60*PI/180, 60*PI/180);
	
	float roll_angle = getAngleErrorRollInHorizontalFlight(0.0, mat); // good in horizontal flight, but better at higher climb/sink rates
	
	//for TESTING HAND LAUNCH
	/*if(mat[0] > 0.12){
		roll_angle = getLineRollAngle(line_dir); // good if high line tension nearly perpendicular to wing
		//roll_angle = getAngleErrorRollInHorizontalFlight(0.0, mat); // good in horizontal flight with line tension not perpendicular to wing, but better at higher climb/sink rates
	}*/
	
	float x_axis_control = - 28 * autopilot->landing.X.P * (desired_roll_angle-roll_angle) - 11 * autopilot->landing.X.D * sensor_data.gyro[0];
	x_axis_control = clamp(x_axis_control, -45, 45);
	y_axis_control = clamp(y_axis_control, -50, 50);
	
	// LINE SPEED CONTROL VIA AIR BRAKE
	float line_speed_error = /*-3.75;//*/0.5*(-line_speed - 7.5);
	airbrake = clamp(line_speed_error * 60/2.5, -60, 60);
	airbrake_compensation_by_elevons = 0.5*60 * fmax(0, 1-fabs((line_speed_error-0.5)*0.4));//0.4 = 1/2.5, -0.5 because airbrake servo has very little effect at first, and because airbrake doesn't fully extend
	//printf("line_speed = %f, d_line = %f, lse = %f, ab = %f, abc = %f\n", line_speed, line_length_derivative, line_speed_error, airbrake, airbrake_compensation_by_elevons);
	/*if(line_length < 20){
		airbrake = AIRBRAKE_ON;
		airbrake_compensation_by_elevons = 0;
	}*/
	
	
	//float airbrake = AIRBRAKE_ON;
	//if (height_error < -5) airbrake = AIRBRAKE_OFF;
	
	int flag =  4;
	if(line_length < 20){
		flag = 3;
	}
	if(line_length < 10){
		flag = 2;
	}
	if(line_length < 5){
		flag = 1;
	}
	
	if(sendCounter == 0){
		sendCounter = (frequencyDivider - 1); // 
		//sendDebuggingData(line_length, airbrake, airbrake_compensation_by_elevons, sensor_data.height, y_axis_control, 2);
		
		sendDebuggingData(line_length, height_error, desired_dive_angle_smooth, sensor_data.height, y_axis_offset, 2); // UP-DOWN control
		
		//sendDebuggingData(line_length, angle_error, roll_angle, roll_angle-desired_roll_angle, x_axis_control, 2); // UP-DOWN control
		//sendDebuggingData(line_length, height_error, desired_dive_angle_smooth, y_axis_offset, y_axis_control, 2); // UP-DOWN control
	}else{
		sendCounter--;
	}
	
	float prop_speed = 0;
	/*if( landing_propeller_counter != -1){
		prop_speed = old_prop_speed;
		landing_propeller_counter ++;
		if(landing_propeller_counter > 100){
			landing_propeller_counter = -1;
			prop_speed = 0;
		}
	}*/
	//printf("prop_speed = %f, landing_propeller_counter = %d\n", prop_speed, landing_propeller_counter);
	//printf("airbrake->brake = %f\n", autopilot->brake);
	initControlData(control_data_out, prop_speed, prop_speed,
		- x_axis_control + abs(x_axis_control)*0.1,
		+ x_axis_control + abs(x_axis_control)*0.1,
		airbrake_compensation_by_elevons + y_axis_control -2 + autopilot->brake - x_axis_control*0.2,
		airbrake_compensation_by_elevons + y_axis_control -2 + autopilot->brake + x_axis_control*0.2,
		airbrake, 0, LINE_TENSION_LANDING); return;
}


void eight_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float timestep_in_s){
	
	// DIRECTION TIMER
	if(query_timer_seconds(autopilot->timer) > autopilot->sideways_flying_time * autopilot->multiplier * clamp(0.02*line_length, 1, 10)){ // IF TIME TO TURN
		//TURN
		autopilot->direction  *= -1;
		autopilot->multiplier = 1;
		autopilot->timer = start_timer();
	}
	
	float* mat = sensor_data.rotation_matrix;
	float* line_dir = sensor_data.line_direction_vector; // is normed
	
	/*
	float weight_of_kite = 2;
	float line_tension = 6;
	float w_by_lt = weight_of_kite / line_tension;
	// adding weight vector to line tension.
	float combined_line_tension_and_weight_vector[3];
	combined_line_tension_and_weight_vector[0] -= w_by_lt*mat[0];
	combined_line_tension_and_weight_vector[1] -= w_by_lt*mat[3];
	combined_line_tension_and_weight_vector[2] -= w_by_lt*mat[6];
	*/
	// LINE ANGLE could alternatively be calculated from orientation and line angle sensor
	float line_angle_from_zenith = safe_acos(1.4*sensor_data.height/(line_length == 0 ? 1.0 : line_length)); //safe_acos(mat[6]); // roll angle of kite = line angle
	
	float roll_angle = getLineRollAngle(line_dir);
	//float roll_angle = getLineRollAngle(combined_line_tension_and_weight_vector);
	
	
	// 1. STAGE P(I)D: line angle -> flight direction
	float angle_diff = autopilot->eight.desired_line_angle_from_zenith - line_angle_from_zenith;
	float target_angle_adjustment = clamp(angle_diff*autopilot->eight.beta_P, -autopilot->eight.target_angle_beta_clamp, 0.5*autopilot->eight.target_angle_beta_clamp); // 3 works well for line angle control, but causes instability. between -pi/4=-0.7... and pi/4=0.7...
	float target_angle = PI*0.5*autopilot->direction*(autopilot->eight.neutral_beta_sideways_flying_angle_fraction + target_angle_adjustment/* 1 means 1.2*90 degrees, 0 means 0 degrees*/);
	setTargetValueActuator(&(autopilot->slowly_changing_target_angle), target_angle);
	stepActuator(&(autopilot->slowly_changing_target_angle), timestep_in_s);
	float slowly_changing_target_angle_local = getValueActuator(&(autopilot->slowly_changing_target_angle));
	
	
	// 2. STAGE P(I)D: flight direction -> neccessary roll angle
	float angleErrorZAxis = getAngleErrorZAxisImproved(0.0, mat, line_dir);
	float z_axis_offset = angleDifferenceWithoutDiscontinuity(angleErrorZAxis, slowly_changing_target_angle_local);
	float neutral_roll_angle = clamp(angleErrorZAxis*line_angle_from_zenith, -0.65, 0.65); // max 37 degrees
	float desired_roll_angle = clamp(neutral_roll_angle + autopilot->eight.roll.P * z_axis_offset - autopilot->eight.roll.D * sensor_data.gyro[2], -70*PI/180, 70*PI/180);
	
	
	// 3. STAGE P(I)D: neccessary roll angle -> aileron deflection
	float z_axis_control = - 28 * autopilot->eight.Z.P * (desired_roll_angle-roll_angle) - 11 * autopilot->eight.Z.D * sensor_data.gyro[0];
	z_axis_control = clamp(z_axis_control, -30, 30);
	
	// ELEVATOR
	float y_axis_control = autopilot->eight.elevator + autopilot->eight.Y.D * sensor_data.gyro[1];
	
	float airbrake = AIRBRAKE_OFF;
	
	if(old_line_length == 0){
		old_line_length = line_length;
	}else{
		float reel_out_speed = (line_length - old_line_length)/timestep_in_s;
		float forceInNewtons = 60;
		powerInWatts = 0.98 * powerInWatts + 0.02 * reel_out_speed * forceInNewtons;
		old_line_length = line_length;
	}
	
	sendDebuggingData(roll_angle, angleErrorZAxis, desired_roll_angle, slowly_changing_target_angle_local, z_axis_control, 1);
	//sendDebuggingData(angleErrorZAxis, getAngleErrorZAxis(0.0, mat), 0, 0, 0, 0);
	initControlData(control_data_out, 0, 0,
		y_axis_control - z_axis_control + abs(z_axis_control)*0.1,
		y_axis_control + z_axis_control + abs(z_axis_control)*0.1,
		0,
		0,
		airbrake, 0, LINE_TENSION_EIGHT); return;
}

static float height_control_smooth = 0;
static float d_h_smooth = 0;

static float z_axis_I = 0;
static float h_I = 0;

void hover_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension){
	
	float* mat = sensor_data.rotation_matrix;
	float* line_dir = sensor_data.line_direction_vector;
	// HEIGHT
	
	float height = sensor_data.height;
	float d_height = sensor_data.d_height;
	d_h_smooth = 0.8*d_h_smooth + 0.2 * d_height;
	
	
	float desired_height = (line_length == 0 ? 1.0 : line_length) * 0.65; // * sin(PI/6.0)
	float height_error = height - desired_height;
	
	//TODO: cleanup all those constants!
	float height_control_normed = clamp(1 - autopilot->hover.H.P * height_error - autopilot->hover.H.D * clamp(d_h_smooth, -2.0, 2.0), 0.75, 1.5);
	
	
	// autopilot is an approximation to the airflow seen by the elevons (propeller airflow + velocity in height direction)
	float normed_airflow = height_control_normed + clamp(d_h_smooth*0.2, -0.8, 0.8); // this latter constant depends highly on the shape of the ailerons. probably needs to be more aggressive with the full wingspan ones.
	
	h_I -= sign(height_error) * 0.005;
	h_I = clamp(h_I, -5, 5);
	
	float height_control = height_control_normed * 45 + h_I;
	if(height_control > height_control_smooth + 0.5)
		height_control_smooth += 0.5;
	else if(height_control < height_control_smooth - 0.5)
		height_control_smooth -= 0.5;
	
	line_length_derivative = 0.9*line_length_derivative + 0.1 * (line_length - old_line_length)*50; //50Hz, i.e. division by timestep delta_t=1/50
	old_line_length = line_length;
	float line_speed_control = clamp((5-line_length_derivative*2.0)*0.03, 0, 0.15);// 10 degrees pitch-up when line-speed = 0, 0 degrees pitch-up when line-speed = 5m/s
	
	// Y-AXIS
	float y_axis_offset = getAngleErrorYAxis(autopilot->y_angle_offset + line_speed_control, mat);
	// normed_airflow at neutral hover position should be between 1 and 1.5 depending on propeller thrust
	// thus 1.0/(normed_airflow*normed_airflow) is roughly between 1 and 0.5
	// if kite goes up with 4m/s -> +0.8 on normed_airflow -> 1.0/(normed_airflow*normed_airflow) between 0.25 and 0.16
	// if kite goes down with 4m/s -> -0.8 on normed airflow -> 1.0/(normed_airflow*normed_airflow) up to 100
	float y_axis_control = (normed_airflow > 0.0001 ? 1.0/(normed_airflow*normed_airflow) : 1.0) * 0.7 * (- 3.8*autopilot->hover.Y.P * y_axis_offset + 0.7*0.4 * autopilot->hover.Y.D * sensor_data.gyro[1]);
	y_axis_control *= 100;
	//y_axis_control -= 20;
	
	// Z-AXIS
	float desired_z_axis_angle = 0;
	if(line_tension == LINE_TENSION_EIGHT){
		//desired_z_axis_angle = 0.75; // 45 degrees to the left
		desired_z_axis_angle = 0.75; // 45 degrees to the left
	}
	float z_axis_offset = getAngleErrorZAxis(desired_z_axis_angle, mat);//getAngleErrorZAxisImproved(0.0, mat, line_dir); // could use getAngleErrorZAxis(), if pitch stays near horizontal.
	float z_axis_control = - autopilot->hover.Z.P * z_axis_offset * 0.195935*0.44 * 1.5 + autopilot->hover.Z.D * sensor_data.gyro[2]*0.017341*2;
	z_axis_I -= sign(z_axis_offset) * 0.01;
	z_axis_I = clamp(z_axis_I, -6, 6);
	
	z_axis_control *=100;
	z_axis_control += z_axis_I;
	
	//if(fabs(y_axis_offset) > 1.5) z_axis_control = 0;
	
	// X-AXIS
	
	float roll_angle = -getLineRollAngle(line_dir) + 0.1;
	float x_axis_control = (normed_airflow > 0.0001 ? 1.0/(normed_airflow*normed_airflow) : 1.0) * (autopilot->hover.X.P * roll_angle + autopilot->hover.X.D * sensor_data.gyro[0]*0.75);
	x_axis_control *= 100;
	
	if(abs(y_axis_offset) > 0.5) {x_axis_control = 0;}
	
	// FOR DEBUGGING
	/*if(autopilot->RC_switch > 0.5){
		x_axis_control += 45 * autopilot->RC_target_angle; // move ailerons directly
	}else{
		z_axis_control += 45 * autopilot->RC_target_angle; // differentiate propeller thrust
	}*/
	old_prop_speed = height_control_smooth;
	// MIXING
	
	height_control_smooth = clamp(height_control_smooth, 0, 60);
	
	float left_elevon = clamp(y_axis_control + x_axis_control, -50, 50);
	float right_elevon = clamp(y_axis_control - x_axis_control, -50, 50);
	float left_prop = height_control_smooth + z_axis_control;
	float right_prop = height_control_smooth - z_axis_control;
	/*
	if(line_tension == LINE_TENSION_EIGHT){
		left_prop = 90;
		right_prop = 90;
		//left_elevon = 10;
		//right_elevon = 10;
		//y_axis_control = -10;
	}
	*/
	sendDebuggingData(sensor_data.height, line_length, height_control_smooth, line_speed_control, z_axis_I, 0);
	initControlData(control_data_out, left_prop, right_prop, left_elevon, right_elevon, left_elevon, right_elevon, AIRBRAKE_OFF, 0, line_tension); return;
	
}
