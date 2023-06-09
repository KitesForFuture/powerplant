"use strict";

class Wind{
	
	constructor(){
		this.windvector = new THREE.Vector3();
		
		this.setSpeedAndDirection(6.5, 0);
	}
	
	update(){
		
	}
	
	setSpeedAndDirection(speed, direction){
		this.direction = direction*Math.PI/180;
		this.speed = speed;
		this.windvector.x = 0;
		this.windvector.y = speed*Math.sin(this.direction);
		this.windvector.z = speed*Math.cos(this.direction);
	}
	
	getWindVector(){
		return this.windvector;
	}
}


