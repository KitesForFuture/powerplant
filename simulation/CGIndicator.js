"use strict";

class CGIndicator extends KitePart{
	
	constructor(rigidBody){
		//super(new THREE.CylinderGeometry( radius, radius, 0.01, 20 ), new THREE.MeshBasicMaterial( { side: THREE.DoubleSide, color: new THREE.Color('red') } ), rigidBody);
		super(new THREE.CylinderGeometry( 0.003, 0.003, 1, 10 ), new THREE.MeshBasicMaterial( { side: THREE.DoubleSide, color: new THREE.Color('red') } ), rigidBody);
		
	}
	
	setScale(y){
		this.scale.y = y;
	}
}
