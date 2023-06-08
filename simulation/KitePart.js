"use strict";

class KitePart extends THREE.Object3D{
	
	constructor(geometry, material, rigidBody){
		super();
		this.mesh = new THREE.Mesh(geometry, material);
		this.scalingObject = new THREE.Object3D();
		this.scalingObject.add(this.mesh);
		this.rotationObject = new THREE.Object3D();
		this.rotationObject.add(this.scalingObject);
		this.add(this.rotationObject);
		this.rigidBody = rigidBody;
		this.mesh.castShadow = true;
		//this.mesh.receiveShadow = true;
	}
	
	//Matrix to go from RigidBody to FlatPlate (inverse of M)
	getN(){
		this.updateWorldMatrix(true, false);
		
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
