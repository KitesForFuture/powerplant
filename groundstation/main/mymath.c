#define true 1
#define false 0

int smallpow(int x, int p){
	int ret = 1;
	for(int i = 0; i < p; i++){
		ret *= x;
	}
	return ret;
}

float norm3(float a[3]){
	return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}
float norm3nonarray(float a, float b, float c){
	return sqrt(a*a + b*b + c*c);
}

void crossProduct(float a, float b, float c, float x, float y, float z, float result[3]){
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

float tmp_mat[9];

void mat_mult(float a[9], float b[9]){
	
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			tmp_mat[3*i+j] = 0;
			for(int k = 0; k < 3; k++){
				tmp_mat[3*i+j] += a[3*i+k]*b[3*k+j];
			}
		}
	}
}

void mat_mult_mat_transp(float a[9], float b[9]){
	
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			tmp_mat[3*i+j] = 0;
			for(int k = 0; k < 3; k++){
				tmp_mat[3*i+j] += a[3*i+k]*b[3*j+k];
			}
		}
	}
}

float tmp_vec[3];
void mat_transp_mult_vec(float mat[9], float a, float b, float c){
	
	for(int i = 0; i < 3; i++){
			tmp_vec[i] = mat[i]*a + mat[i+3]*b + mat[i+6]*c;
			
	}
}

// currently not used
void mat_mult_vec(float mat[9], float a, float b, float c){
	
	for(int i = 0; i < 3; i++){
			tmp_vec[i] = mat[i*3]*a + mat[i*3+1]*b + mat[i*3+2]*c;
			
	}
}

void array_copy(float a[9], float b[9]){
	for(int i = 0; i < 9; i++){
		a[i] = b[i];
	}
}

void normalize_matrix(float a[9]){
	
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
float tmp_rot_matrix[9];
void rotate_towards_g(float mat[], float a_init, float b_init, float c_init, float a, float b, float c){
	// mat'*(a_init, b_init, c_init)'
	mat_transp_mult_vec(mat, a_init, b_init, c_init);
	
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
	
	// rotation matrix
	tmp_rot_matrix[0] = 1;
	tmp_rot_matrix[1] = -axis_3*sin(angle);
	tmp_rot_matrix[2] = axis_2*sin(angle);
	tmp_rot_matrix[3] = axis_3*sin(angle);
	tmp_rot_matrix[4] = 1;
	tmp_rot_matrix[5] = -axis_1*sin(angle);
	tmp_rot_matrix[6] = -axis_2*sin(angle);
	tmp_rot_matrix[7] = axis_1*sin(angle);
	tmp_rot_matrix[8] = 1;
	
	mat_mult_mat_transp(mat, tmp_rot_matrix);
}


// FUNCTIONS NECESSARY FOR REINFORCEMENT LEARNING

// multiply column vector by row vector, result is a matrix
void vectorMultiply(float a[], float b[], float destination[], int lengthA, int lengthB){
	for(int i = 0; i < lengthA; i++){
		for(int j = 0; j < lengthB; j++){
			destination[i*lengthB + j] = a[i]*b[j];
		}
	}
}

// scalar product of two vectors
float scalarProductOfMatrices(float A[], float B[], int length){
	float ret = 0;
	for(int i = 0; i < length; i++){
		ret += A[i]*B[i];
	}
	return ret;
}

// multiply vector with a scalar, i.e. scale a vector
void scalarMult(float alpha, float source[], float destination[], int length){
	for(int i = 0; i < length; i++){
		destination[i] = alpha*source[i];
	}
}

void copyVector(float source[], float destination[], int length){
	for(int i = 0; i < length; i++){
		destination[i] = source[i];
	}
}

void normalize(float source[], float destination[], int length){
	float norm = sqrt(scalarProductOfMatrices(source, source, length));
	if(norm > 0.000000000000001){
		scalarMult(1.0/norm, source, destination, length);
	}else{
		copyVector(source, destination, length);
	}
}

void addMatrices(float A[], float B[], float destination[], int length){
	for(int i = 0; i < length; i++){
		destination[i] = A[i]+B[i];
	}
}

void subtractMatrices(float A[], float B[], float destination[], int length){
	for(int i = 0; i < length; i++){
		destination[i] = A[i]-B[i];
	}
}

//returns random number between 0 and 1
float myrandom(){
	return ((float)rand())/((float)RAND_MAX);
}
