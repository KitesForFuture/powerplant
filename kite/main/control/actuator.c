#include "../../../common/helpers/math.h"
#include "actuator.h"

void initActuator(Actuator* actuator, float speed, float minValue, float maxValue){
	
	actuator->targetValue = minValue;
	actuator->speed = speed;
	actuator->minValue = minValue;
	actuator->maxValue = maxValue;
	actuator->currentValue = (minValue+maxValue)*0.5;
}

void stepActuator(Actuator* actuator, float time_difference){
	
	if(actuator->targetValue > actuator->currentValue){
		actuator->currentValue += time_difference * actuator->speed;
		actuator->currentValue = clamp(actuator->currentValue, actuator->minValue, actuator->targetValue);// avoid shooting over targetValue
	}else{
		actuator->currentValue -= time_difference * actuator->speed;
		actuator->currentValue = clamp(actuator->currentValue, actuator->targetValue, actuator->maxValue);// avoid shooting under targetValue
	}
	actuator->currentValue = clamp(actuator->currentValue, actuator->minValue, actuator->maxValue);
	
}

float getValueActuator(Actuator* actuator){
	
	return actuator->currentValue;
	
}

void setTargetValueActuator(Actuator* actuator, float value){
	
	actuator->targetValue = value;
}

