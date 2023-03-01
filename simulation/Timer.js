"use strict";

class Timer{
	
	constructor(){
		this.reset();
	}
	
	timeElapsedInSeconds(){
		let d = new Date();
		return 0.001*(d.getTime() - this.start_time);
	}
	
	reset(){
		let d = new Date();
		this.start_time = d.getTime(); // ms
	}
}
