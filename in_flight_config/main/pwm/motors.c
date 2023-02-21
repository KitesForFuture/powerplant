#include "motors.h"

static uint32_t escDegree2pulsewidth(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (ESC_MIN_PULSEWIDTH + (((ESC_MAX_PULSEWIDTH - ESC_MIN_PULSEWIDTH) * (degree_of_rotation)) / (ESC_MAX_DEGREE)));
    return cal_pulsewidth;
}

static uint32_t servoDegree2pulsewidth(int32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation+90)) / (180)));
    return cal_pulsewidth;
}

void initMotors(int pins[], int length){
	//Set GPIO 18 as PWM0A, to which the servo is connected
	//MCPWM_UNIT_0 and MCPWM_UNIT_1 are the two pwm units, each capable of controlling 3 motors
	//MCPWM0A, MCPWM0B, MCPWM1A, MCPWM1B, MCPWM2A, MCPWM2B are the possible outputs
	//in each MCPWM_UNIT there are 3 operators (0,1,2 number is associated with timer) each containing 2 generators (A and B)
	
	//SO TOTAL NUMBER OF simple one input signal MOTORS PER UNIT IS 6 and TOTAL NUMBER OF MOTORS PER ESP32 is thus 12
	
	//SYNC_X (for timer), FAULT_X, CAP_X ("CAP"=capture), X=0,1,2 are the possible inputs for the three timers 0,1,2
	//18 is the pin number written on the board. can be any gpio-pin.
	
    if(length > 0) mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, pins[0]);
    if(length > 1) mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, pins[1]);
    if(length > 2) mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, pins[2]);
    if(length > 3) mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0B, pins[3]);
    if(length > 4) mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, pins[4]);
    
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 200;
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);
}

// angle: -90 ... 90
void setAngle(int motor, float degreeF){
	int32_t degree = (int32_t)degreeF;
	//if(degree > 90 || degree < -90) return; //replaced with continuous extension, i.e. if(deg > 90) deg = 90; ...
	if(degree > 90) degree = 90;
	if(degree < -90) degree = -90;
	mcpwm_unit_t unit = MCPWM_UNIT_0;
	mcpwm_generator_t gen = MCPWM_OPR_A;
	mcpwm_timer_t timer = MCPWM_TIMER_0;
	
	switch(motor) {
		case 0: break;
		case 1: gen = MCPWM_OPR_B; break;
		case 2: unit = MCPWM_UNIT_1; break;
		case 3: unit = MCPWM_UNIT_1; gen = MCPWM_OPR_B; break;
		case 4: unit = MCPWM_UNIT_0; gen = MCPWM_OPR_A; timer = MCPWM_TIMER_1; break;
		default: break;
	}
	//MCPWM_TIMER_0 automatically sets the output to MCPWM0X?
	mcpwm_set_duty_in_us(unit, timer, gen, servoDegree2pulsewidth(degree));
	//mcpwm_set_duty() to set duty in % instead of microseconds
}

// degree: 0 ... 90
void setSpeed(int motor, float degreeF){
	if(degreeF > 90){degreeF = 90;}
	if(degreeF < 0){degreeF = 0;}
	uint32_t degree = (uint32_t)degreeF;
	mcpwm_unit_t unit = MCPWM_UNIT_0;
	mcpwm_generator_t gen = MCPWM_OPR_A;
	mcpwm_timer_t timer = MCPWM_TIMER_0;
	switch(motor) {
		case 0: break;
		case 1: gen = MCPWM_OPR_B; break;
		case 2: unit = MCPWM_UNIT_1; break;
		case 3: unit = MCPWM_UNIT_1; gen = MCPWM_OPR_B; break;
		case 4: unit = MCPWM_UNIT_0; gen = MCPWM_OPR_A; timer = MCPWM_TIMER_1; break;
		default: break;
	}
	//MCPWM_TIMER_0 automatically sets the output to MCPWM0X?
	mcpwm_set_duty_in_us(unit, timer, gen, escDegree2pulsewidth(degree));
	//mcpwm_set_duty() to set duty in % instead of microseconds
}
