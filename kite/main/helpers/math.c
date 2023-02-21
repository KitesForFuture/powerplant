#include <math.h>
#include "math.h"
// acos function continuously extended beyond -1 and 1.

float safe_acos(float number_more_or_less_between_one_and_minus_one){
	if(number_more_or_less_between_one_and_minus_one <= -1) return 3.14159265;
	if(number_more_or_less_between_one_and_minus_one >= 1) return 0;
	return acos(number_more_or_less_between_one_and_minus_one);
}

float safe_asin(float number_more_or_less_between_one_and_minus_one){
	if(number_more_or_less_between_one_and_minus_one <= -1) return -1.5708;
	if(number_more_or_less_between_one_and_minus_one >= 1) return 1.5708;
	return asin(number_more_or_less_between_one_and_minus_one);
}

int smallpow(int x, int p){
	int ret = 1;
	for(int i = 0; i < p; i++){
		ret *= x;
	}
	return ret;
}

void crossProduct(float a, float b, float c, float x, float y, float z, float result[]){
	result[0] = b*z-c*y;
	result[1] = c*x-a*z;
	result[2] = a*y-b*x;
}

float sign(float x){
	if(x < 0)
		return -1;
	else
		return 1;
}

void mat_mult(float a[], float b[], float out[]){
	
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			out[3*i+j] = 0;
			for(int k = 0; k < 3; k++){
				out[3*i+j] += a[3*i+k]*b[3*k+j];
			}
		}
	}
}

void mat_mult_mat_transp(float a[], float b[], float out[]){
	
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			out[3*i+j] = 0;
			for(int k = 0; k < 3; k++){
				out[3*i+j] += a[3*i+k]*b[3*j+k];
			}
		}
	}
}

void mat_transp_mult_vec(float mat[], float a, float b, float c, float out[]){
	
	for(int i = 0; i < 3; i++){
			out[i] = mat[i]*a + mat[i+3]*b + mat[i+6]*c;
			
	}
}

void mat_mult_vec(float mat[], float a, float b, float c, float out[]){
	
	for(int i = 0; i < 3; i++){
			out[i] = mat[i*3]*a + mat[i*3+1]*b + mat[i*3+2]*c;
			
	}
}

void normalize_matrix(float a[]){
	
	// Gram-Schmidt orthogonalization
	// (direction of first column stays constant and always only the latter two are rotated)
	
	float norm = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
	for(int i = 0; i < 3; i++){
		a[i] /= norm;
	}
	
	float scalarProd = a[0]*a[3]+a[1]*a[4]+a[2]*a[5];
	for(int i = 0; i < 3; i++){
		a[3+i] -= scalarProd*a[i];
	}
	
	norm = sqrt(a[3]*a[3] + a[4]*a[4] + a[5]*a[5]);
	for(int i = 0; i < 3; i++){
		a[3+i] /= norm;
	}
	
	scalarProd = a[0]*a[6]+a[1]*a[7]+a[2]*a[8];
	for(int i = 0; i < 3; i++){
		a[6+i] -= scalarProd*a[i];
	}
	
	scalarProd = a[3]*a[6]+a[4]*a[7]+a[5]*a[8];
	for(int i = 0; i < 3; i++){
		a[6+i] -= scalarProd*a[3+i];
	}
	
	norm = sqrt(a[6]*a[6] + a[7]*a[7] + a[8]*a[8]);
	for(int i = 0; i < 3; i++){
		a[6+i] /= norm;
	}
}

// rotates matrix mat such that mat'*(a_init, b_init, c_init)' aligns more with (a,b,c)'
// (a_init, b_init, c_init) can be initially measured acceleration vector, usually something close to (0,0,1)
// (a,b,c) can be the currently measured acceleration vector
void rotate_towards_g(float mat[], float a_init, float b_init, float c_init, float a, float b, float c, float out[]){
	// mat'*(a_init, b_init, c_init)'
	float tmp_vec[3];
	mat_transp_mult_vec(mat, a_init, b_init, c_init, tmp_vec);
	
	// determine the normalized rotation axis mat'*(a_init, b_init, c_init)' x (a,b,c)'
	float axis_1 = tmp_vec[1]*c-tmp_vec[2]*b;
	float axis_2 = tmp_vec[2]*a-tmp_vec[0]*c;
	float axis_3 = tmp_vec[0]*b-tmp_vec[1]*a;
	float norm = sqrt(axis_1*axis_1 + axis_2*axis_2 + axis_3*axis_3);
	axis_1 /= norm;
	axis_2 /= norm;
	axis_3 /= norm;
	
	// determine the approximate angle between mat'*(a_init, b_init, c_init)' and (a,b,c)'
	float differenceNorm = sqrt((mat[2]-a)*(mat[2]-a) + (mat[5]-b)*(mat[5]-b) + (mat[8]-c)*(mat[8]-c));
	// multiply by small number, so we move only tiny bit in right direction at every step -> averaging measured acceleration from vibration
	float angle = differenceNorm*0.001;//When connected to USB, then 0.00004 suffices. When autonomous on battery 0.0004 (10 times larger) does just fine.
	// 0.00004 works, error 0.0004
	// 0.0004 works, error 0.002 except in battery mode
	// 0.004 works, error 0.01
	// 0.04, error 0.07
	// it appears that the gyro drifts a lot more when powered on battery instead of USB.
	// ToDoLeo constants / knowledge inside calcualtion.
	
	// rotation matrix
	float tmp_rot_matrix[9];
	tmp_rot_matrix[0] = 1;
	tmp_rot_matrix[1] = -axis_3*sin(angle);
	tmp_rot_matrix[2] = axis_2*sin(angle);
	tmp_rot_matrix[3] = axis_3*sin(angle);
	tmp_rot_matrix[4] = 1;
	tmp_rot_matrix[5] = -axis_1*sin(angle);
	tmp_rot_matrix[6] = -axis_2*sin(angle);
	tmp_rot_matrix[7] = axis_1*sin(angle);
	tmp_rot_matrix[8] = 1;
	
	mat_mult_mat_transp(mat, tmp_rot_matrix, out);
}


// FUNCTIONS NECESSARY FOR REINFORCEMENT LEARNING

// scalar product of two vectors
float scalarProductOfMatrices(float A[], float B[], int length){
	float ret = 0;
	for(int i = 0; i < length; i++){
		ret += A[i]*B[i];
	}
	return ret;
}

float normalize(float a[], int length) {
	float norm = sqrt(scalarProductOfMatrices(a, a, length));
	if(norm > 0.00001){
		for(int i = 0; i < length; i++){
			a[i] = (1.0/norm)*a[i];
		}
	}
	
	return norm;
}

float angle(float vec1[3], float vec2[3]){
	
	normalize(vec1, 3);
	normalize(vec2, 3);
	
	return safe_acos(scalarProductOfMatrices(vec1, vec2, 3));
}
/*
void setup_slowly_changing_angle(float* target_angle_delta_timer, float* slowly_changing_target_angle){
	*target_angle_delta_timer = 0;
	*slowly_changing_target_angle = 0;
}*/
// replaced by actuator.c
/*
void get_slowly_changing_angle(float target_angle, float turning_speed, Time* target_angle_delta_timer, float* slowly_changing_target_angle){
	
    //static float slowly_changing_target_angle = 0;
    
    
    float d_t = get_time_step(target_angle_delta_timer);
    if(*slowly_changing_target_angle < target_angle){
    	*slowly_changing_target_angle += d_t * turning_speed;
    	if(*slowly_changing_target_angle > target_angle) *slowly_changing_target_angle = target_angle;
    }
    if(*slowly_changing_target_angle > target_angle){
    	*slowly_changing_target_angle -= d_t * turning_speed;
    	if(*slowly_changing_target_angle < target_angle) *slowly_changing_target_angle = target_angle;
    }
    //return slowly_changing_target_angle;
}
*/
float clamp(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}
