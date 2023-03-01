"use strict";

class Actuator{
	
	constructor(speed, minValue, maxValue){
		this.targetValue = 0;
		this.speed = speed;
		this.minValue = minValue;
		this.maxValue = maxValue;
		this.currentValue = 0;
	}
	
	step(time_difference){
		if(this.targetValue > this.currentValue){
			this.currentValue += time_difference * this.speed;
			this.currentValue = clamp(this.currentValue, this.minValue, this.targetValue);
		}else{
			this.currentValue -= time_difference * this.speed;
			this.currentValue = clamp(this.currentValue, this.targetValue, this.maxValue);
		}
		this.currentValue = clamp(this.currentValue, this.minValue, this.maxValue);
	}
	
	getValue(){
		return this.currentValue;
	}
	
	setTargetValue(tV){
		this.targetValue = tV;
	}
}
