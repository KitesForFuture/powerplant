"use strict";

class KitePart extends THREE.Mesh{
	
	constructor(geometry, material, rigidBody){
		super(geometry, material);
		this.rigidBody = rigidBody;
	}
	
	//Matrix to go from RigidBody to FlatPlate (inverse of M)
	getN(){
		
		var mat = this.matrix.clone();
		var ancestor = this.parent;
		while(!(ancestor === this.rigidBody)){
			mat = mat.premultiply(ancestor.matrix);
			ancestor = ancestor.parent;
		}
		return mat.elements;
	}
	
	getForceAndCentreOfPressureInKiteCoords(){
		var N = this.getN();
		
		//in rigidBody coordinates
		return new ForceData(0, new THREE.Vector3(N[12], N[13], N[14]));
	}
}
