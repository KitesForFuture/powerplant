"use strict";

class KiteVisualization extends Visualization{
	
	constructor(canvasName, kite, wind){
		
		super(canvasName);
		
		// GROUND PLANE
		this.groundPlane = new THREE.Mesh(new THREE.BoxBufferGeometry(0.2, 300, 300), new THREE.MeshBasicMaterial( { color: new THREE.Color(0x30a000),transparent: true, opacity: 0.8 } ) );
		this.groundPlane.position.x = -0.1;
		
		for(let i = -10; i < 10; i++){
			for(let j = -10; j < 10; j++){
				let tree = new THREE.Mesh(new THREE.SphereBufferGeometry(0.5), new THREE.MeshBasicMaterial( { color: new THREE.Color('red') } ) );
				tree.position.y = 15*j;
				tree.position.z = 15*i;
				this.scene.add(tree);
			}
		}
		
		
		this.cameraAttachedToKite = true;
		
		this.kite = kite;
		this.wind = wind;
		
		// KITE SMOKE AND PATH
		this.smoke = new Smoke(15, 'yellow');
		this.path = new Smoke(300, 'black');
		this.pathIndex = 0;
		
		// KITE LINE
		this.kitelineMesh = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('grey') }));
		this.kitelineMeshLeft = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('grey') }));
		this.kitelineMeshRight = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('grey') }));
		
		// WIND VECTOR
		this.windVector = new THREE.Object3D();
		this.windVector.position.x = 0.5;
		this.windVectorLine = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('yellow') }));
		this.windVectorLine.rotation.y = -Math.PI/2;
		this.windVectorArrowHalf = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('yellow') }));
		this.windVectorArrowOtherHalf = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('yellow') }));
		this.windVectorArrowHalf.position.z = 1;
		this.windVectorArrowOtherHalf.position.z = 1;
		this.windVectorArrowHalf.rotation.y = -Math.PI/2;
		this.windVectorArrowHalf.rotation.x = 0.8*Math.PI;
		this.windVectorArrowOtherHalf.rotation.y = -Math.PI/2;
		this.windVectorArrowOtherHalf.rotation.x = -0.8*Math.PI;
		this.windVectorArrowHalf.scale.x = 0.2;
		this.windVectorArrowOtherHalf.scale.x = 0.2;
		this.windVector.add(this.windVectorLine);
		this.windVector.add(this.windVectorArrowHalf);
		this.windVector.add(this.windVectorArrowOtherHalf);
		this.updateWindVectorDirection();
		
		this.scene.add(this.windVector);
		this.scene.add(this.kitelineMesh);
		this.scene.add(this.kitelineMeshLeft);
		this.scene.add(this.kitelineMeshRight);
		this.scene.add(this.kite);
		this.scene.add(this.groundPlane);
		this.scene.add(this.smoke);
		this.scene.add(this.path);
		
		this.scene.add(testBall);
	}
	
	updateWindVectorDirection(){
		this.windVector.rotation.x = -this.wind.direction;
		this.windVector.scale.y = this.wind.speed;
		this.windVector.scale.z = this.wind.speed;
	}
	
	updateKiteLineVisualization(){
		//calculate line knot
		var lineKnot = this.kite.getPositionOfLineKnot();
		// update kite line mesh
		let mat = new THREE.Matrix4();
		mat.set(
			lineKnot.x,   0, 0,  0,
			lineKnot.y,   1, 0,  0,
			lineKnot.z,   0, 1,  0,
			0,    0, 0,  1
		);
		this.kitelineMesh.matrixAutoUpdate = false;
		this.kitelineMesh.matrix = mat;
		
		
		var tblm = this.kite.tannenbaumLeft.matrixWorld.elements;
		var factor = this.kite.tannenbaum_length;
		var z_displacement = new THREE.Vector3(tblm[8], tblm[9], tblm[10]);
		z_displacement.normalize();
		z_displacement.multiplyScalar(-factor*0.5);
		var leftAttachment = new THREE.Vector3(tblm[12], tblm[13], tblm[14]);
		leftAttachment.add(z_displacement);
		let matLeft = new THREE.Matrix4();
		
		//testBall.position.copy(leftAttachment);
		
		matLeft.set(
			leftAttachment.x-lineKnot.x,   0, 0,  lineKnot.x,
			leftAttachment.y-lineKnot.y,   1, 0,  lineKnot.y,
			leftAttachment.z-lineKnot.z,   0, 1,  lineKnot.z,
			0,    0, 0,  1
		);
		this.kitelineMeshLeft.matrixAutoUpdate = false;
		this.kitelineMeshLeft.matrix = matLeft;
		
		
		var tbrm = this.kite.tannenbaumRight.matrixWorld.elements;
		z_displacement = new THREE.Vector3(tblm[8], tblm[9], tblm[10]);
		z_displacement.normalize();
		z_displacement.multiplyScalar(-factor*0.5);
		var rightAttachment = new THREE.Vector3(tbrm[12], tbrm[13], tbrm[14]);
		rightAttachment.add(z_displacement);
		let matRight = new THREE.Matrix4();
		matRight.set(
			rightAttachment.x-lineKnot.x,   0, 0,  lineKnot.x,
			rightAttachment.y-lineKnot.y,   1, 0,  lineKnot.y,
			rightAttachment.z-lineKnot.z,   0, 1,  lineKnot.z,
			0,    0, 0,  1
		);
		this.kitelineMeshRight.matrixAutoUpdate = false;
		this.kitelineMeshRight.matrix = matRight;
	}
	
	updateSmoke(timestep_in_s){
		this.smoke.setScale(0.2/this.camera.zoom);
		this.path.setScale(0.2/this.camera.zoom);
		
		if(timestep_in_s == 0) return;
		
		// SMOKE
		for(let i = 1; i < this.smoke.spheres.length; i++){
			this.smoke.spheres[i-1].position.copy( this.smoke.spheres[i].position.add(this.wind.getWindVector().clone().multiplyScalar(timestep_in_s)) );
		}
		this.smoke.spheres[this.smoke.spheres.length-1].position.copy( this.kite.positionR.clone() );
		
		// PATH
		
		this.path.spheres[this.pathIndex].position.copy( this.kite.positionR.clone() );
		this.pathIndex++;
		if(this.pathIndex >= this.path.spheres.length) this.pathIndex = 0;
		
		/*
		for(let i = 1; i < this.path.spheres.length; i++){
			this.path.spheres[i-1].position.copy( this.path.spheres[i].position);
		}
		this.path.spheres[this.path.spheres.length-1].position.copy( this.kite.positionR.clone() );
		*/
	}
	
	update(timestep_in_s){
		// OPACITY OF THE GROUND PLANE FROM BELOW
		if(this.diagram2.rotation.y < 0) this.groundPlane.material.opacity = 0.9; else this.groundPlane.material.opacity = 0.2;
		this.updateWindVectorDirection();
		this.updateKiteLineVisualization();
		this.updateSmoke(timestep_in_s);
		if(this.cameraAttachedToKite){
			this.diagram.position.copy(this.kite.positionR);
		}else{
			this.diagram.position.set(0,0,0);
		}
		
	}
}
