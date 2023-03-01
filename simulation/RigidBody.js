"use strict";

class RigidBody extends THREE.Object3D{
	
	constructor(){
		super();
		
		this.mass = 1;
		
		this.positionR = new THREE.Vector3(0, 0, 0);
		
		this.velocity = new THREE.Vector3(0, 0, 0);
		
		this.angular_inertia = new THREE.Vector3(1, 1, 1);
		
		this.rotation_matrix = new THREE.Matrix3();
		this.rotation_matrix.set(
			0,   0, 1,
			0,   1, 0,
			-1,   0, 0
		);
		
		this.angular_velocity = new THREE.Vector3(0, 0, 0);
		
		this.counter = 0;
	}
	
	//TODO: do this properly without relying on the "tmp" matrix
	kite2world(vector){
		return vector.applyMatrix3(this.rotation_matrix);
	}
	
	world2kite(vector){
		return vector.applyMatrix3(this.rotation_matrix.clone().transpose());
	}
	
	sub_update(force, torque, timestep){
		// "F=m*a" discrete integration
		var acceleration = force.clone().divideScalar(this.mass);
		this.velocity.add(acceleration.multiplyScalar(timestep));
		this.positionR.add(this.velocity.clone().multiplyScalar(timestep));
		
		// Euler's equations (rigid body dynamics)
		var angularAcceleration = new THREE.Vector3();
		angularAcceleration.x = torque.x/this.angular_inertia.x - this.angular_velocity.y*this.angular_velocity.z*(this.angular_inertia.z - this.angular_inertia.y)/this.angular_inertia.x
		angularAcceleration.y = torque.y/this.angular_inertia.y - this.angular_velocity.x*this.angular_velocity.z*(this.angular_inertia.x - this.angular_inertia.z)/this.angular_inertia.y
		angularAcceleration.z = torque.z/this.angular_inertia.z - this.angular_velocity.y*this.angular_velocity.x*(this.angular_inertia.y - this.angular_inertia.x)/this.angular_inertia.z
		
		this.counter++;
		if(this.counter > 1/timestep){
			//console.log(angularAcceleration);
			this.counter = 0;
		}
		this.angular_velocity.add(angularAcceleration.multiplyScalar(timestep));
		//infinitesimal rotation matrix
		var d_rotation_matrix = new THREE.Matrix3();
		d_rotation_matrix.set(
			1,							-this.angular_velocity.z*timestep,	this.angular_velocity.y*timestep,
			this.angular_velocity.z*timestep,	1,							-this.angular_velocity.x*timestep,
			-this.angular_velocity.y*timestep,	this.angular_velocity.x*timestep,	1
		);
		this.rotation_matrix.multiply(d_rotation_matrix); // multiply is correct. premultiply would wrongly rotate around world coord system axes instead.
		normalize_matrix(this.rotation_matrix);
		
	}
	
	update(force/*in world coordinates*/, torque/*in kite coordinates*/, timestep/*in seconds*/){
		
		//divide the timestep to increase precision
		var timestep_multiplier = 10;
		for(var i = 0; i < timestep_multiplier; i++){
			this.sub_update(force, torque, timestep/timestep_multiplier);
		}
		
		var m = this.rotation_matrix.elements;
		var d = this.positionR;
		
		this.matrixAutoUpdate = false;
		this.matrix.set(m[0],	m[3],	m[6],	d.x,
						m[1],	m[4],	m[7],	d.y,
						m[2],	m[5],	m[8],	d.z,
						0,		0,		0,		1);
	}
	
	getSensorData(){
		return new SensorData(this.rotation_matrix.clone(), this.angular_velocity.clone(), this.positionR.x, this.velocity.x);
	}
}


