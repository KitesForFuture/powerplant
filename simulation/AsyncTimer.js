"use strict";

var globalTime = 0.0;

function advanceGlobalTime(delta_time_in_s){
	globalTime += delta_time_in_s;
}

class AsyncTimer extends Timer{
	
	timeElapsedInSeconds(){
		return globalTime - this.start_time;
	}
	
	reset(){
		this.start_time = globalTime;
	}
}
