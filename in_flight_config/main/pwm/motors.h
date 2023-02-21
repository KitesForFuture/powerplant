//You can get these values from the datasheet of the servo you use, in general pulse width varies between 1000 to 2000 microsecond
#define SERVO_MIN_PULSEWIDTH 500//400 //Minimum pulse width in microsecond (500 according to nettigo.eu)
#define SERVO_MAX_PULSEWIDTH 2500//2400 //Maximum pulse width in microsecond
#define ESC_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond
#define ESC_MAX_PULSEWIDTH 2000 //Maximum pulse width in microsecond
#define ESC_MAX_DEGREE 90 //Maximum angle in degree upto which servo can rotate


#include "driver/mcpwm.h"


// 26,27,12,13 is a good choice
void initMotors(int pins[], int length);

// angle: -90 ... 90
void setAngle(int motor, float degreeF);

// degree: 0 ... 90
void setSpeed(int motor, float degreeF);
