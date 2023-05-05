#include <stdio.h>

struct PID
{
	float P;
	float I;
	float D;
};

float control(struct PID pid){
    printf("%f %f %f\n", pid.P, pid.I, pid.D);
}

const int y = 42;

void test(int y){
    printf("%d\n", y);
}

void main(){
    test(5);
}

