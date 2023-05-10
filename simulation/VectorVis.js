"use strict";

class VectorVis extends THREE.Object3D{
	//new THREE.Color('yellow')
	constructor(colorArg){
		
		super();
		//this.position.x = 0.5;
		this.lineGeometry3 = new THREE.BufferGeometry();
		let pOsitions2 = [];
		pOsitions2.push(0, 0, 0.0);
		pOsitions2.push(1, 0, 0.0);
		this.lineGeometry3.addAttribute('position', new THREE.Float32BufferAttribute(pOsitions2, 3));
		
		this.windVectorLine = new THREE.Line(this.lineGeometry3, new THREE.LineBasicMaterial({ color: colorArg }));
		//this.windVectorLine.rotation.y = -Math.PI/2;
		
		/*this.windVectorArrowHalf = new THREE.Line(this.lineGeometry3, new THREE.LineBasicMaterial({ color: new THREE.Color('yellow') }));
		this.windVectorArrowOtherHalf = new THREE.Line(this.lineGeometry3, new THREE.LineBasicMaterial({ color: new THREE.Color('yellow') }));
		this.windVectorArrowHalf.position.z = 1;
		this.windVectorArrowOtherHalf.position.z = 1;
		this.windVectorArrowHalf.rotation.y = -Math.PI/2;
		this.windVectorArrowHalf.rotation.x = 0.8*Math.PI;
		this.windVectorArrowOtherHalf.rotation.y = -Math.PI/2;
		this.windVectorArrowOtherHalf.rotation.x = -0.8*Math.PI;
		this.windVectorArrowHalf.scale.x = 0.2;
		this.windVectorArrowOtherHalf.scale.x = 0.2;
		*/
		this.add(this.windVectorLine);
		//this.windVector.add(this.windVectorArrowHalf);
		//this.windVector.add(this.windVectorArrowOtherHalf);
		
		//this.updateWindVectorDirection();
		
	}
	
	set(pos, force){
		//this.position.set(vec);
		//this.lookAt(vec);
		//this.scale.x = this.scale.y = 0.05*vec.length();
		//console.log(vec);
		//this.matrix.lookAt(new Vector3(0, 0, 0), vec, new Vector3(1, 0, 0));
		let mat = new THREE.Matrix4();
		let factor = 0.05;
		mat.set(
			factor*force.x,	0, 0,  pos.x,
			factor*force.y,	1, 0,  pos.y,
			factor*force.z,	0, 1,  pos.z,
			0,    		0, 0,  1
		);
		this.matrixAutoUpdate = false;
		this.matrix = mat;
	}
}
