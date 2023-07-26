
#ifndef HELPERS_KITEMATH
#define HELPERS_KITEMATH

#include <math.h>

#include <stdio.h>
//#include "../helpers/timer.h"

#define PI 3.14159265
 
float safe_acos(float number_more_or_less_between_one_and_minus_one);
float safe_asin(float number_more_or_less_between_one_and_minus_one);
float angle(float vec1[3], float vec2[3]);
float norm(float a[], int length);
float signed_angle2(float vec1[2], float vec2[2]);
float signed_angle3(float vec1[3], float vec2[3]);
void cross(float vec1[3], float vec2[3], float result[3]);

int smallpow(int x, int p);

void crossProduct(float a, float b, float c, float x, float y, float z, float result[]);

float sign(float x);

void mat_mult(float a[], float b[], float out[]);

void mat_mult_mat_transp(float a[], float b[], float out[]);

void mat_transp_mult_vec(float mat[], float a, float b, float c, float out[]);

void mat_mult_vec(float mat[], float a, float b, float c, float out[]);

void normalize_matrix(float a[]);

void rotate_towards_g(float mat[], float a_init, float b_init, float c_init, float a, float b, float c, float out[]);
void rotate_towards_gravity_and_north(float mat[], float mag_a_init, float mag_b_init, float mag_c_init, float mag_a, float mag_b, float mag_c, float a_init, float b_init, float c_init, float a, float b, float c, float out[]);

float scalarProductOfMatrices(float A[], float B[], int length);

float normalize(float a[], int length);

// replaced by actuator.c
//void get_slowly_changing_angle(float target_angle, float turning_speed, Time* target_angle_delta_timer, float* slowly_changing_target_angle);

float clamp(float d, float min, float max);

//TODO-Ben:
//void get_column(float matrix[], float column[], int column_number);

#endif
