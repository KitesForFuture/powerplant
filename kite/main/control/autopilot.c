#include "../../../common/helpers/math.h"
#include "../../../common/helpers/timer.h"
#include "autopilot.h"

#define DATA_MODE 1

void sendDebuggingData(float num1, float num2, float num3, float num4, float num5, float num6);


void loadConfigVariables(Autopilot* autopilot, float* config_values){
	autopilot->hover.Y.P = config_values[14];
	autopilot->hover.Y.D = config_values[15];
	
	autopilot->hover.Z.P = config_values[17];
	autopilot->hover.Z.D = config_values[18];
	
	autopilot->hover.X.D = config_values[19];
	
	autopilot->hover.H.P = config_values[20];
	autopilot->hover.H.D = config_values[21];
	autopilot->hover.y_angle_offset = config_values[16]*PI/180;
	autopilot->transition_y_angle_offset = config_values[31]*PI/180;
	autopilot->eight.Y.D = config_values[29];
	
	autopilot->eight.Z.P = config_values[27];
	autopilot->eight.Z.D = config_values[28];
	
	autopilot->landing.X.P = config_values[25];
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
	
	autopilot->eight.Y.P = 1;
	autopilot->landing.X.D = 1;
	
	
	autopilot->y_angle_offset = autopilot->hover.y_angle_offset;
	
	autopilot->mode = HOVER_MODE;
	autopilot->direction = 1;
	autopilot->multiplier = FIRST_TURN_MULTIPLIER;
	
	initActuator(&(autopilot->slowly_changing_target_angle), autopilot->turning_speed, -3.14, 3.14);
	
	autopilot->timer = 0;
}

void stepAutopilot(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension){
	
	if(autopilot->fm == 3.0){ // 3.0 is VESC final landing mode
		autopilot->mode = LANDING_MODE;
	}
	
	float timestep_in_s = 0.02; // 50 Hz, but TODO: MUST measure more precisely!
	if(autopilot->mode == AIRPLANE_MODE){
		landing_control(autopilot, control_data_out, sensor_data, line_length, line_tension, false); return;
	}
	
	if(autopilot->mode == HOVER_MODE){
		if(autopilot->fm != 0.0){ // 0.0 is VESC launch mode
			autopilot->mode = HOVER_EIGHT_TRANSITION;
			autopilot->timer = start_timer();
		}
		autopilot->y_angle_offset = autopilot->hover.y_angle_offset;
		
		hover_control(autopilot, control_data_out, sensor_data, line_length, LINE_TENSION_LAUNCH); return;
	}else if(autopilot->mode == HOVER_EIGHT_TRANSITION){
		if(query_timer_seconds(autopilot->timer) > 1){
			autopilot->timer = start_timer();
			autopilot->multiplier = FIRST_TURN_MULTIPLIER;
			autopilot->mode = EIGHT_MODE;
			autopilot->y_angle_offset = autopilot->hover.y_angle_offset;
		}
		autopilot->y_angle_offset = autopilot->transition_y_angle_offset;
		hover_control(autopilot, control_data_out, sensor_data, line_length, LINE_TENSION_EIGHT); return;
	}else if(autopilot->mode == EIGHT_MODE){
		if(autopilot->fm == 2.0){//landing mode request from VESC
			autopilot->mode = LANDING_MODE;
		}
		eight_control(autopilot, control_data_out, sensor_data, line_length, timestep_in_s); return;
	}else if(autopilot->mode == LANDING_MODE){
		if(autopilot->fm == 1.0){ // 1.0 is VESC eight mode
			autopilot->mode = LANDING_EIGHT_TRANSITION;
		}
		landing_control(autopilot, control_data_out, sensor_data, line_length, line_tension, false); return;
	}else if(autopilot->mode == LANDING_EIGHT_TRANSITION){
		if(sensor_data.rotation_matrix[0] > 0.1){
			autopilot->timer = start_timer();
			//autopilot->direction = 1;
			autopilot->multiplier = FIRST_TURN_MULTIPLIER;
			autopilot->mode = EIGHT_MODE;
		}
		return landing_control(autopilot, control_data_out, sensor_data, line_length, line_tension, true);
	}
}

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

float getAngleErrorZAxis(float offset, float mat[9]){
	float controllable_axis[3] = {mat[6], mat[7], mat[8]};
	float axis_we_wish_horizontal[3] = {mat[3], mat[4], mat[5]};
	return getAngleError(offset, controllable_axis, axis_we_wish_horizontal);
}

float getAngleErrorYAxis(float offset, float mat[9]){
	float controllable_axis[3] = {mat[3], mat[4], mat[5]};
	float axis_we_wish_horizontal[3] = {-mat[6], -mat[7], -mat[8]};
	return getAngleError(offset, controllable_axis, axis_we_wish_horizontal);
}

static float desired_dive_angle_smooth = 0;

void landing_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension, int transition){
	float* mat = sensor_data.rotation_matrix;
	
	// HEIGHT CONTROL
	float height = sensor_data.height-autopilot->landing.desired_height;
	float height_error = clamp(height - line_length*0.2 /* 20 percent descent slope*/, -2, 10);
	
	float desired_dive_angle = 0.15*autopilot->landing.dive_angle_P*height_error;//-desired_line_angle - 2.0 * line_angle_error;
	desired_dive_angle_smooth = 0.8 * desired_dive_angle_smooth + 0.2 * desired_dive_angle;
	
	desired_dive_angle_smooth = clamp( desired_dive_angle_smooth, -PI/10, PI/6 );
	
	if(transition){
		desired_dive_angle_smooth = -PI/6;
	}
	
	float y_axis_offset = -getAngleErrorYAxis(-desired_dive_angle_smooth - PI/2, mat);
	float y_axis_control = 3 - 3*15.0 * autopilot->landing.Y.P * y_axis_offset - 7 * 0.5 * 0.66 * autopilot->landing.Y.D * sensor_data.gyro[1];
	
	float x_axis_control = -100 * mat[3] * autopilot->landing.X.P;// - 0*50*autopilot->landing.X.D * sensor_data.gyro[0];
	
	autopilot->brake = clamp(autopilot->brake, 0, 35);
	
	sendDebuggingData(height, line_length, height_error, desired_dive_angle_smooth, y_axis_offset, y_axis_control);
	initControlData(control_data_out, 0, 0, autopilot->brake - y_axis_control-1*x_axis_control, autopilot->brake - y_axis_control+1*x_axis_control, -autopilot->brake, 0, LINE_TENSION_LANDING); return;
}

void eight_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float timestep_in_s){

	float* mat = sensor_data.rotation_matrix;
	if(query_timer_seconds(autopilot->timer) > autopilot->sideways_flying_time * autopilot->multiplier * clamp(0.02*sensor_data.height, 1, 10)){ // IF TIME TO TURN
		//TURN
		autopilot->direction  *= -1;
		autopilot->multiplier = 1;
		autopilot->timer = start_timer();
	}
	float z_axis_angle_from_zenith = safe_acos(mat[6]); // roll angle of kite = line angle
	
	float angle_diff = autopilot->eight.desired_line_angle_from_zenith - z_axis_angle_from_zenith;
	float target_angle_adjustment = clamp(angle_diff*autopilot->eight.beta_P, -autopilot->eight.target_angle_beta_clamp, autopilot->eight.target_angle_beta_clamp); // 3 works well for line angle control, but causes instability. between -pi/4=-0.7... and pi/4=0.7...
	
	float target_angle = PI*0.5*autopilot->direction*(autopilot->eight.neutral_beta_sideways_flying_angle_fraction + target_angle_adjustment/* 1 means 1.2*90 degrees, 0 means 0 degrees*/);
	setTargetValueActuator(&(autopilot->slowly_changing_target_angle), target_angle);
	stepActuator(&(autopilot->slowly_changing_target_angle), timestep_in_s);
	float slowly_changing_target_angle_local = getValueActuator(&(autopilot->slowly_changing_target_angle));
	
	float z_axis_offset = getAngleErrorZAxis(0.0, mat);
	z_axis_offset -= slowly_changing_target_angle_local;
	float z_axis_control = - 0.56 * autopilot->eight.Z.P * z_axis_offset + 0.22 * autopilot->eight.Z.D * sensor_data.gyro[2];
	z_axis_control *=100;
	
	// ELEVATOR
	float y_axis_control = autopilot->eight.elevator - 1 * autopilot->eight.Y.D * sensor_data.gyro[1];
	
	sendDebuggingData(sensor_data.height, z_axis_angle_from_zenith, target_angle_adjustment, slowly_changing_target_angle_local, z_axis_offset, z_axis_control);
	initControlData(control_data_out, 0, 0, y_axis_control - 0.5*z_axis_control, y_axis_control + 0.5*z_axis_control, 0, 0, LINE_TENSION_EIGHT); return;
}

void hover_control(Autopilot* autopilot, ControlData* control_data_out, SensorData sensor_data, float line_length, float line_tension){
	
	float* mat = sensor_data.rotation_matrix;
	// HEIGHT
	
	float d_height = sensor_data.d_height;
	
	float line_angle = safe_asin(sensor_data.height/(line_length == 0 ? 1.0 : line_length));
	
	//TODO: cleanup all those constants!
	float height_control_normed = clamp(0.92 - 1.15*5.8*autopilot->hover.H.P * (line_angle-PI/6.0) - 1.15*autopilot->hover.H.D * clamp(d_height, -1.0, 1.0), 0.7, 1.5);
	
	// autopilot is an approximation to the airflow seen by the elevons (propeller airflow + velocity in height direction)
	float normed_airflow = height_control_normed + clamp(sensor_data.d_height*0.2, -0.8, 0.8); // this latter constant depends highly on the shape of the ailerons. probably needs to be more aggressive with the full wingspan ones.
	
	float height_control = height_control_normed * 55.901;
	
	// Y-AXIS
	
	float y_axis_offset = getAngleErrorYAxis(autopilot->y_angle_offset, mat);
	// normed_airflow at neutral hover position should be between 1 and 1.5 depending on propeller thrust
	// thus 1.0/(normed_airflow*normed_airflow) is roughly between 1 and 0.5
	// if kite goes up with 4m/s -> +0.8 on normed_airflow -> 1.0/(normed_airflow*normed_airflow) between 0.25 and 0.16
	// if kite goes down with 4m/s -> -0.8 on normed airflow -> 1.0/(normed_airflow*normed_airflow) up to 100
	float y_axis_control = (normed_airflow > 0.0001 ? 1.0/(normed_airflow*normed_airflow) : 1.0) * 0.7 * (- 3.8*autopilot->hover.Y.P * y_axis_offset + 0.7*0.4 * autopilot->hover.Y.D * sensor_data.gyro[1]);
	y_axis_control *= 100;
	
	// Z-AXIS
	
	float z_axis_offset = getAngleErrorZAxis(0.0, mat);
	float z_axis_control = - autopilot->hover.Z.P * z_axis_offset * 0.195935*0.44 * 1.5 + autopilot->hover.Z.D * sensor_data.gyro[2]*0.017341*2;
	z_axis_control *=100;
	if(fabs(y_axis_offset) > 1.5) z_axis_control = 0;
	// X-AXIS
	
	float x_axis_control = (normed_airflow > 0.0001 ? 1.0/(normed_airflow*normed_airflow) : 1.0) * autopilot->hover.X.D * sensor_data.gyro[0]*0.75;
	x_axis_control *= 100;
	
	// MIXING
	
	height_control = clamp(height_control, 0, 50);
	
	float left_prop = height_control + z_axis_control;
	float right_prop = height_control - z_axis_control;
	float left_elevon = y_axis_control + x_axis_control;
	float right_elevon = y_axis_control - x_axis_control;
	
	if(line_tension == LINE_TENSION_EIGHT){
		left_prop = 70;
		right_prop = 70;
		left_elevon = 10;
		right_elevon = 10;
		y_axis_control = 0;
	}
	
	sendDebuggingData(sensor_data.height, 0, 0, 0, 0, 0);
	initControlData(control_data_out, left_prop, right_prop, left_elevon, right_elevon, (left_elevon+right_elevon)*0.125, 0, line_tension); return;
	
}
