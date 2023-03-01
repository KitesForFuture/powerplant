"use strict";

const HOVER_MODE = 0;
const EIGHT_MODE = 1;
const TRANSITION_MODE = 2;
const STRAIGHT_MODE = 3;
const LANDING_MODE = 4;
const LANDING_MODE_HOVER = 5;
const LANDING_EIGHT_TRANSITION = 6;
const FINAL_LANDING_MODE = 7;
const FINAL_LANDING_MODE_HOVER = 8;


const FIRST_TURN_MULTIPLIER = 0.5;

const LEFT = 0;
const RIGHT = 1;

class Autopilot{
	constructor(){
		
		this.hover = new Object();
		
		this.hover.Y = new Object();
		this.hover.Y.P = 1;
		this.hover.Y.D = 1;
		
		this.hover.Z = new Object();
		this.hover.Z.P = 1;
		this.hover.Z.D = 1;
		
		this.hover.X = new Object();
		this.hover.X.D = 1;
		
		this.hover.H = new Object();
		this.hover.H.P = 1;
		this.hover.H.D = 1;
		
		this.y_angle_offset = 0.15;
		this.desired_height = 0;
		
		this.figure_eight = new Object(); // ...
		
		this.mode = HOVER_MODE;
		this.direction = 1;
		this.sideways_flying_time = 7;
		
		this.multiplier = FIRST_TURN_MULTIPLIER;
		this.turning_speed = 1.3;//0.75;//0.75;
		this.slowly_changing_target_angle = new SlowlyChangingAngle(this.turning_speed, -1000, 1000);
		
		this.old_line_length = 0;
		this.smooth_reel_in_speed = 0.1;
		
		this.timer = new AsyncTimer();
	}
	
	step(sensor_data, line_length, line_tension, timestep_in_s){
		
		advanceGlobalTime(timestep_in_s);
		
		if(this.mode == HOVER_MODE){
			//if(sensor_data.height > 20){
			//	this.mode = LANDING_MODE;
			//}
			if(sensor_data.height > 50){
				this.mode = TRANSITION_MODE;
				this.timer.reset();
			}
			this.y_angle_offset = 0.15;
			return this.hover_control(sensor_data, line_length, 1);
		}else if(this.mode == TRANSITION_MODE){
			if(this.timer.timeElapsedInSeconds() > 1){ // 3s
				this.timer.reset();
				this.multiplier = FIRST_TURN_MULTIPLIER;
				this.mode = EIGHT_MODE;//LANDING_MODE;//EIGHT_MODE;
				this.y_angle_offset = 0.15;
				//this.mode = LANDING_MODE;//TODO: remove
			}
			//return this.transition_control(sensor_data, line_length);
			this.y_angle_offset = -0.15;
			return this.hover_control(sensor_data, line_length, 25);
			//return this.transition_control(sensor_data, line_length);
			
		}else if(this.mode == EIGHT_MODE){
			if(sensor_data.height > 130){
				this.mode = LANDING_MODE;
				
			}
			return this.eight_control(sensor_data, line_length, timestep_in_s);
		}else if(this.mode == LANDING_MODE){
			if(sensor_data.height < 60){
				this.mode = LANDING_EIGHT_TRANSITION;//EIGHT_MODE;
			}
			return this.landing_control(sensor_data, line_length, line_tension, false);
		}else if(this.mode == FINAL_LANDING_MODE){
			
			if(line_length < this.old_line_length){
				this.smooth_reel_in_speed = 0.8*this.smooth_reel_in_speed - 0.2*(line_length - this.old_line_length)/timestep_in_s;
			}
			this.old_line_length = line_length;
			if(line_length / this.smooth_reel_in_speed < 0.5){
				console.log("l_l = " + line_length + ", reel_speed = " + this.smooth_reel_in_speed);
				this.smooth_reel_in_speed = 0.1;
				this.mode = FINAL_LANDING_MODE_HOVER;
			}
			return this.landing_control(sensor_data, line_length, line_tension, false);
		}else if(this.mode == FINAL_LANDING_MODE_HOVER){
			// aerodynamic braking
			return new ControlData(0, 0, 45, 45, 0, 2);
			//return this.landing_control(sensor_data, line_length, true);
		}else if(this.mode == LANDING_EIGHT_TRANSITION){
			if(sensor_data.rotation_matrix.elements[0] > 0.1){
				this.timer.reset();
				//this.direction = 1;
				this.multiplier = FIRST_TURN_MULTIPLIER;
				this.mode = EIGHT_MODE;
			}
			return this.landing_control(sensor_data, line_length, line_tension, true);
		}//else if(this.mode == LANDING_MODE_HOVER){
		//	return this.hover_control(sensor_data, line_length, 25);
		//}
	}
	
	landing_control(sensor_data, line_length, line_tension, transition){
		let mat = sensor_data.rotation_matrix.elements;
		let line_angle = asin_clamp(sensor_data.height/line_length);//correct
		
		let desired_line_angle = Math.PI/4 * 0.75;//0.3;
		
		let line_angle_error = line_angle-desired_line_angle;// negative -> too low, positive -> too high
		let desired_dive_angle = -desired_line_angle - 2 * line_angle_error;
		desired_dive_angle = clamp(desired_dive_angle, -Math.PI*0.5*0.8, 0);
		if(transition){
			desired_dive_angle = Math.PI/4;
		}
		//console.log(desired_dive_angle * 180 / Math.PI);
		
		let y_axis_offset = this.getAngleError(desired_dive_angle - Math.PI/2, new THREE.Vector3(mat[3], mat[4], mat[5]), new THREE.Vector3(-mat[6], -mat[7], -mat[8]));
		//TODO: bei weniger wind (geringe Seilspannung) sollte das hier mit faktor multipliziert werden, um genügend Steuerwirkung zu haben
		let y_axis_control = /*this.hover.Y.P**/10*(- 0.24*3.8*2*0.5* y_axis_offset + 0.0027*0.7*0.4 *0.2*0.5 * sensor_data.gyro.y);
		//console.log(line_tension);
		y_axis_control *= line_tension < 5 ? 5 : 1;
		
		
		//let y_axis_control = -100*(line_angle-Math.PI/4)*this.hover.Y.P;// + 0.01*this.hover.Y.D * sensor_data.gyro.y;
		let x_axis_control = -50 * (mat[3] * this.hover.Z.P + 0*this.hover.Z.D * sensor_data.gyro.x);
		//console.log(y_axis_control);
		//x_axis_control *= 100;
		
		return new ControlData(0, 0, y_axis_control-1*x_axis_control, y_axis_control+1*x_axis_control, x_axis_control, 2);
	}
	
	eight_control(sensor_data, line_length, timestep_in_s){
		var mat = sensor_data.rotation_matrix.elements;
		console.log("time = " + this.timer.timeElapsedInSeconds() + ", target = " + this.sideways_flying_time + ", mult = " + this.multiplier);
		if(this.timer.timeElapsedInSeconds() > this.sideways_flying_time * this.multiplier){ // IF TIME TO TURN
			//TURN
			console.log("turn");
			this.direction  *= -1;
			this.multiplier = 1;
			this.timer.reset();
		}
		let z_axis_angle = acos_clamp(mat[6]); // roll angle of kite = line angle
		this.loggingString = "Seilwinkel = " + (90 - z_axis_angle *180 / Math.PI).toFixed(0) + " deg<br>"
		let RC_requested_line_angle = Math.PI/4 * 1.5; // TODO: rename line_angle
		this.loggingString += "Sollseilwinkel = " + (90 - RC_requested_line_angle *180 / Math.PI).toFixed(0) + " deg<br>"
		let angle_diff = RC_requested_line_angle - z_axis_angle;
		let target_angle_adjustment = angle_diff*3; // 3 works well for line angle control, but causes instability. between -pi/4=-0.7... and pi/4=0.7...
		if(target_angle_adjustment > 0.4) target_angle_adjustment = 0.4;
		if(target_angle_adjustment < -0.4) target_angle_adjustment = -0.4;
		this.loggingString += "clamp(angle_diff * 0.5) = " + target_angle_adjustment.toFixed(3) + "<br>";
		let sideways_flying_angle_fraction = 0.9;//0.9;//0.75; // fraction of 90 degrees, this influences the angle to the horizon, smaller => greater angle = flying higher
		let target_angle = Math.PI*0.5*this.direction*(sideways_flying_angle_fraction + target_angle_adjustment/* 1 means 1.2*90 degrees, 0 means 0 degrees*/);
		this.loggingString += "target_angle = " + (target_angle * 180 / Math.PI).toFixed(0) + " deg<br>";
		this.slowly_changing_target_angle.setTargetValue(target_angle);
		this.slowly_changing_target_angle.step(timestep_in_s);
		let slowly_changing_target_angle = this.slowly_changing_target_angle.getValue();//(target_angle, this.turning_speed);
		
		var z_axis_offset = this.getAngleError(0.0, new THREE.Vector3(mat[6], mat[7], mat[8]), new THREE.Vector3(mat[3], mat[4], mat[5]));
		z_axis_offset -= slowly_changing_target_angle;
		var z_axis_control = - 1 * z_axis_offset + 1 * sensor_data.gyro.z;
		z_axis_control *=100;
		//console.log("target_angle = " + target_angle + ", slch = " + slowly_changing_target_angle + ", z_control = " + z_axis_control);
		//rudder_angle = getRudderControl(target_angle, slowly_changing_target_angle, (float)(pow(5,CH5)), (float)(pow(5,CH5))); //TODO: CH5,CH6 here for P/D
		//var y_axis_control = 10 * this.hover.Y.D * sensor_data.gyro.y;
		
		//let y_axis_control = 15;
		var y_axis_control = 15 - 1 * this.hover.Y.D * sensor_data.gyro.y;
		return new ControlData(0, 0, y_axis_control - 0.5*z_axis_control, y_axis_control + 0.5*z_axis_control, 0, 35);
		//return new ControlData(0, 0, y_axis_control - 0.5*z_axis_control, y_axis_control + 0.5*z_axis_control, z_axis_control, 35);
	}
	
	/*transition_control(sensor_data, line_length){
		return new ControlData(60, 60, 0, 0, 30, 25); // works somehow
		//return new ControlData(90, 90, 10, 10, 0, 0);
	}*/
	straight_control(sensor_data, line_length){
		return new ControlData(0, 0, 20, 20, 0, 25);
	}
	
	
	hover_control(sensor_data, line_length, line_tension){
		
		var mat = sensor_data.rotation_matrix.elements;
		
		// HEIGHT
		
		var height = sensor_data.height;
		var d_height = sensor_data.d_height;
		
		var line_angle = asin_clamp(sensor_data.height/line_length);
		
		//TODO: cleanup all those constants!
		var height_control_normed = 1.15*clamp(0.8 - 5.8*this.hover.H.P * (line_angle-Math.PI/4) - this.hover.H.D * clamp(d_height, -1, 1), 0.8, 1.2);
		
		// this is an approximation to the airflow seen by the elevons (propeller airflow + velocity in height direction)
		var normed_airflow = height_control_normed + sensor_data.d_height*7*1.15/8/5;
		
		var height_control = height_control_normed * 90*5/7/1.15;
		//TODO: investigate this:
		//height_control *= 0.7*mat[0] + 0.3; // decrease propeller thrust when nose not pointing straight up
		//if(mat[0] < 0) height_control = 0; // propellers off when nose pointing down
		
		// Y-AXIS
		
		var y_axis_offset = this.getAngleError(this.y_angle_offset, new THREE.Vector3(mat[3], mat[4], mat[5]), new THREE.Vector3(-mat[6], -mat[7], -mat[8]));
		var y_axis_control = (normed_airflow)**-2 * 0.7 * (- 3.8*this.hover.Y.P * y_axis_offset + 0.7*0.4 * this.hover.Y.D * sensor_data.gyro.y);
		y_axis_control *= 100;
		
		// Z-AXIS
		
		var z_axis_offset = this.getAngleError(0.0, new THREE.Vector3(mat[6], mat[7], mat[8]), new THREE.Vector3(mat[3], mat[4], mat[5]));
		var z_axis_control = - this.hover.Z.P * z_axis_offset + this.hover.Z.D * sensor_data.gyro.z;
		z_axis_control *=100;
		
		// X-AXIS
		
		var x_axis_control = this.hover.X.D * sensor_data.gyro.x;
		x_axis_control *= 100;
		
		// MIXING
		
		var left_elevon = y_axis_control + x_axis_control;
		var right_elevon = y_axis_control - x_axis_control;
		var left_prop = height_control + z_axis_control;
		var right_prop = height_control - z_axis_control;
		var rudder = z_axis_control; // just to move it out of the way, so it doesn't provide drag for the wind
		
		return new ControlData(left_prop, right_prop, left_elevon, right_elevon, rudder, line_tension);
	}
	
	getAngleError(offset, controllable_axis/*vector*/, axis_we_wish_horizontal/*vector*/){
		var where_axis_we_wish_horizontal_should_be = controllable_axis.clone().cross(new THREE.Vector3(1,0,0));
		
		if(where_axis_we_wish_horizontal_should_be < 0.05){ // close to undefinedness
			return 0;
		}else{
			where_axis_we_wish_horizontal_should_be.normalize();
			return Math.sign(axis_we_wish_horizontal.x) * angle(axis_we_wish_horizontal, where_axis_we_wish_horizontal_should_be) - offset;
		}
	}
}
