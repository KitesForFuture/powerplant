"use strict";

class Smoke extends THREE.Object3D{
	
	constructor(size, colorname){
		super();
		
		this.spheres = [];
		this.spheres.length = size;
		this.numSpheres = size;
		
		for(var i = 0; i < size; i++){
			this.spheres[i] = new THREE.Mesh(new THREE.SphereBufferGeometry(0.2, 3, 2), new THREE.MeshBasicMaterial( { color: new THREE.Color(colorname) } ) );
			this.add(this.spheres[i]);
		}
	}
}
