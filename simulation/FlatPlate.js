"use strict";

class FlatPlate extends KitePart{
	
	constructor(x, y, z, rigidBody, colorName, camber){
		super(new THREE.BoxBufferGeometry(1, 1, 1), new THREE.MeshBasicMaterial( { color: new THREE.Color(colorName) } ), rigidBody);
		
		this.dimensions = new THREE.Vector3(1, 1, 1);
		this.setDimensions(x, y, z);
		
		this.additionalAirflow = new THREE.Vector3(0,0,0);
		
		this.camber = camber;
		
		this.type = "FlatPlate";
		this.torqueVis = new VectorVis(new THREE.Color('red'));
		
		this.AoA_for_vis = 0;
	}
	
	setDimensions(x, y, z){
		this.scalingObject.scale.x = x;
		this.dimensions.x = x;
		this.scalingObject.scale.y = y;
		this.dimensions.y = y;
		this.scalingObject.scale.z = z;
		this.dimensions.z = z;
	}
	
	setDim(x){
		this.scalingObject.scale.x = x;
		this.dimensions.x = x;
	}
	setDimY(y){
		this.scalingObject.scale.y = y;
		this.dimensions.y = y;
	}
	setDimZ(z){
		this.scalingObject.scale.z = z;
		this.dimensions.z = z;
	}
	
	setAdditionalWind(vec){
		this.additionalAirflow.copy(vec);
	}
	
	// velocity of flat plate in world coordinates
	getVelocity(){
		// M = Matrix_kite * Matrix_flatPlate^-1
		var N = this.getN();
		
		// - translation part in RigidBody Coordinates
		var e = new THREE.Vector3(-N[12], -N[13], -N[14]);
		//rotation induced velocity in RigidBody Coordinates
		var v = e.cross(this.rigidBody.angular_velocity);
		var rot = getRotationMatrixPart(this.rigidBody.matrixWorld);
		// (velocity of rigidBody) - ((relative Position of FlatPlate) x (angular_velocity of rigidBody)).inWorldCoordinates
		var vel = this.rigidBody.velocity.clone().add(v.applyMatrix3(rot));
		return vel;
	}
	
	getAngularVelocities(){
		// M = Matrix_kite * Matrix_flatPlate^-1
		var M = this.getM();
		// (rotation Part of M) * (angular_velocity of rigidBody)
		return this.rigidBody.angular_velocity.clone().applyMatrix3(new THREE.Matrix3(m[0], m[4], m[8], m[1], m[5], m[9], m[2], m[6], m[10]));
	}
	
	getForceAndCentreOfPressureInKiteCoords(wind_vector_in_global_coordinates){
		let forceData = super.getForceAndCentreOfPressureInKiteCoords();
		
		// USEFUL VARIABLES
		//in world coordinates
		var velocity = this.getVelocity();
		var relative_wind = wind_vector_in_global_coordinates.clone().add(this.additionalAirflow).sub(velocity);// so far ignoring the cancelling effect when wind minus velocity comes close to propeller airflow speed
		var relative_wind_speed = relative_wind.length();
		var relative_wind_normalized = relative_wind.normalize();
		
		var airfoil_normal = new THREE.Vector3(this.matrixWorld.elements[8], this.matrixWorld.elements[9], this.matrixWorld.elements[10]);
		var angle_of_attack_var = angle_of_attack(airfoil_normal, relative_wind_normalized);
		this.AoA_for_vis = angle_of_attack_var;
		//FORCE
		var direction_of_lift = lift_direction(airfoil_normal, relative_wind_normalized);
		var direction_of_drag = drag_direction(relative_wind_normalized);
		var c_l = lift_coefficient(angle_of_attack_var, this);
		var c_d = drag_coefficient(angle_of_attack_var, 2*this.dimensions.y/this.dimensions.x, this);
		var lift_vector_in_world_coordinates = direction_of_lift.multiplyScalar(lift(c_l, relative_wind_speed, this.dimensions.x * this.dimensions.y));
		var drag_vector_in_world_coordinates = direction_of_drag.multiplyScalar(drag(c_d, relative_wind_speed, this.dimensions.x * this.dimensions.y));
		forceData.force = lift_vector_in_world_coordinates.add(drag_vector_in_world_coordinates);
		
		
		// CENTRE OF PRESSURE
		let dir = projectToComplementOfNormedVector(relative_wind_normalized, airfoil_normal).normalize(); // all in global coordinates
		
		//x and y coordinates of the flat plate
		let x = new THREE.Vector3(this.matrixWorld.elements[0], this.matrixWorld.elements[1], this.matrixWorld.elements[2]);
		let y = new THREE.Vector3(this.matrixWorld.elements[4], this.matrixWorld.elements[5], this.matrixWorld.elements[6]);
		
		let x_projection = projectToNormedVector(dir, x).length();
		let y_projection = projectToNormedVector(dir, y).length();
		
		let alpha = Math.PI*0.5;
		
		if(Math.abs(this.dimensions.x * y_projection) > 0.0001){
			//TODO: alpha is not correct, it seems
			alpha = Math.atan( this.dimensions.y * x_projection / (this.dimensions.x * y_projection) );
		}
		
		// l must be between this.dimensions.x and this.dimensions.y
		let l = Math.sqrt(this.dimensions.y * this.dimensions.y * Math.cos(alpha)*Math.cos(alpha) + this.dimensions.x * this.dimensions.x * Math.sin(alpha)*Math.sin(alpha));
		
		if(this.camber){
		
			l*=0.25;
			
			let multiplier = (0.5+0.5*Math.cos(2*angle_of_attack_var));
			
			l*= (0.5+0.5*Math.cos(2*angle_of_attack_var));
			
			dir.multiplyScalar(-l);
			
			forceData.pressure_centre.add(this.rigidBody.world2kite(dir)); // in rigid body (kite) coordinates
		}
		
		return forceData;
	}
}
