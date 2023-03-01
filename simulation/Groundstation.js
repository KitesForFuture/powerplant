"use strict";

class Groundstation{
	
	constructor(min8Tension, max8Tension, minLandingTension, maxLandingTension, minReelInSpeed, maxReelInSpeed, c_l_by_c_d, A){
	
		this.currentValue = 0;
		
		this.min8Tension = min8Tension;
		
		this.max8Tension = max8Tension;
		
		this.minLandingTension = minLandingTension;
		
		this.maxLandingTension = maxLandingTension;
		
		this.minReelInSpeed = minReelInSpeed;
		
		this.maxReelInSpeed = maxReelInSpeed;
		
		this.c_l_by_c_d = c_l_by_c_d;
		
		this.A = A;
	}
	
	getLineTension(reel_out_speed){
	
		let c_l = 1;
		
		if(reel_out_speed > 0){ // REEL OUT
			
			let tension = this.A * 1.2*0.5*c_l*(2*reel_out_speed*this.c_l_by_c_d)**2;
			
			return clamp(tension, this.min8Tension, this.max8Tension);
			
		}else{ // REEL IN
		
			let a = (this.maxLandingTension - this.minLandingTension)/((-this.minReelInSpeed) - (-this.maxReelInSpeed)); // a is positive
			
			let tension = this.maxLandingTension + a * (reel_out_speed - (-this.minReelInSpeed));
			
			return clamp(tension, this.minLandingTension, this.maxLandingTension);
		}
	}
	
}
