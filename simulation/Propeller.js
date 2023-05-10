"use strict";

class Propeller extends KitePart{
	
	constructor(radius, rigidBody, maxThrustInNewton){
		//super(new THREE.CylinderGeometry( radius, radius, 0.01, 20 ), new THREE.MeshBasicMaterial( { side: THREE.DoubleSide, color: new THREE.Color('red') } ), rigidBody);
		super(new THREE.BoxBufferGeometry(radius*2, 0.001, 0.02), new THREE.MeshBasicMaterial( { side: THREE.DoubleSide, color: new THREE.Color('red') } ), rigidBody);
		
		
		this.thrust = 0;
		this.maxThrustInNewton = maxThrustInNewton;
		
	}
	
	setThrust(th){
		this.thrust = th * this.maxThrustInNewton/90.0; // max is 8N of thrust
	}
	
	/*getForce(){
		if(this.thrust == 0) this.fold();
		else this.unfold();
		// so far ignoring cancelling effect of relative velocity of kite!
		var propeller_direction_in_world_coordinates = new THREE.Vector3(this.matrixWorld.elements[4], this.matrixWorld.elements[5], this.matrixWorld.elements[6]);
		return propeller_direction_in_world_coordinates.multiplyScalar(this.thrust);
	}*/
	
	getForceAndCentreOfPressureInKiteCoords(){
		if(this.thrust == 0) this.fold();
		else this.unfold();
		// so far ignoring cancelling effect of relative velocity of kite!
		var propeller_direction_in_world_coordinates = new THREE.Vector3(this.matrixWorld.elements[4], this.matrixWorld.elements[5], this.matrixWorld.elements[6]);
		
		let forceData = super.getForceAndCentreOfPressureInKiteCoords();
		forceData.force = propeller_direction_in_world_coordinates.multiplyScalar(this.thrust);
		
		return forceData;
	}
	
	update(){
		if(this.thrust != 0){
			this.rotation.x += 1;
		}
	}
	
	fold(){
		this.scale.x = 0.2;
		this.scale.y = 10;
		this.scale.z = 0.2;
	}
	
	unfold(){
		this.scale.x = 1;
		this.scale.y = 1;
		this.scale.z = 1;
	}
}
