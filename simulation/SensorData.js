"use strict";

class SensorData{
	
	constructor(rotation_matrix, rotation_matrix_line, gyro, height, d_height){
		this.rotation_matrix = rotation_matrix;
		this.rotation_matrix_line = rotation_matrix_line;
		this.gyro = gyro;
		this.height = height;
		this.d_height = d_height;
	}
	
}
