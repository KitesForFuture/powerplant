#ifndef CONTROLS_ACTUATOR
#define CONTROLS_ACTUATOR

struct _Actuator {
	float targetValue;
	float speed;
	float minValue;
	float maxValue;
	float currentValue;
};
typedef struct _Actuator Actuator;

void initActuator(Actuator* actuator, float speed, float minValue, float maxValue);

void stepActuator(Actuator* actuator, float time_difference);

float getValueActuator(Actuator* actuator);

void setTargetValueActuator(Actuator* actuator, float value);

#endif
