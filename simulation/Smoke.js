"use strict";

class Smoke extends THREE.Object3D{
	
	constructor(size, colorname){
		super();
		
		this.spheres = [];
		this.spheres.length = size;
		this.numSpheres = size;
		
		for(var i = 0; i < size; i++){
			let mesh = new THREE.Mesh(new THREE.SphereBufferGeometry(1, 3, 2), new THREE.MeshBasicMaterial( { color: new THREE.Color(colorname) } ) );
			this.spheres[i] = new THREE.Object3D();
			this.spheres[i].add(mesh);
			this.add(this.spheres[i]);
		}
		this.setScale(1);
	}
	
	setScale(scale){
		for(var i = 0; i < this.numSpheres; i++){
			this.spheres[i].scale.x = scale*0.05;
			this.spheres[i].scale.y = scale*0.05;
			this.spheres[i].scale.z = scale*0.05;
		}
	}
}
