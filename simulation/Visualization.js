"use strict";

var SphereGeometry = new THREE.SphereBufferGeometry( 3, 3, 2 );

class Visualization{
	
	toggleCameraPerspective(){
		this.cameraIsOrtho = !this.cameraIsOrtho;
		if(this.cameraIsOrtho){
			this.diagram2.remove(this.camera);
			this.diagram2.add(this.cameraOrtho);
		}else{
			this.diagram2.remove(this.cameraOrtho);
			this.diagram2.add(this.camera);
		}
	}
	
	constructor(canvasName){
		
		this.pauseCallback = function(){};
		this.beginFF = function(){};
		this.endFF = function(){};
		this.beginSpeed = function(){};
		this.endSpeed = function(){};
		// CAMERA SETUP
		
		this.cameraIsOrtho = false;
		
		this.cameraOrtho = new THREE.OrthographicCamera(-2, 2, 1.5, -1.5, -580, 580);
		this.cameraOrtho.rotation.x = -Math.PI;
		this.cameraOrtho.rotation.z = -Math.PI/2;
		//this.camera.position.z = -80;
		this.cameraOrtho.zoom = 0.5;
		this.cameraOrtho.updateProjectionMatrix();
		
		this.camera = new THREE.PerspectiveCamera( 45, 4 / 3, 0.1, 1000 );//new THREE.OrthographicCamera(-2, 2, 1.5, -1.5, -580, 580);
		this.camera.position.z = -10;
		this.camera.rotation.x = -Math.PI;
		this.camera.rotation.z = -Math.PI/2;
		//this.camera.position.z = -80;
		//this.camera.zoom = 0.5;
		this.camera.updateProjectionMatrix();
		
		// LIGHTS
		
		const hemiLight = new THREE.HemisphereLight( 0xffffff, 0xffffff, 1 );
		hemiLight.color.setRGB( 0.8, 0.8, 1 );
		hemiLight.groundColor.setRGB( 0.8, 0.8, 1 );
		hemiLight.position.set( 0, 50, 0 );
		//hemiLight.rotation.x = Math.PI*0.5;
				
				

		this.dirLight = new THREE.DirectionalLight( 0xffffff, 1 );
		this.dirLight.color.setRGB( 0.9, 0.9, 0.7 );
		this.dirLight.position.set( -10, -5, 5 );
		//scene.add( dirLight );

		this.dirLight.castShadow = true;


		this.dirLight.shadow.mapSize.width = 2048;
		this.dirLight.shadow.mapSize.height = 2048;

		const d = 2;

		this.dirLight.shadow.camera.left = - d;
		this.dirLight.shadow.camera.right = d;
		this.dirLight.shadow.camera.top = d;
		this.dirLight.shadow.camera.bottom = - d;

		this.dirLight.shadow.camera.far = 3500;
		this.dirLight.shadow.bias = - 0.0001;
		
		//dirLight.target = 

		//const dirLightHelper = new THREE.DirectionalLightHelper( dirLight, 10 );
		
		// COORDINATE SYSTEM GEOMETRY
		
		this.lineGeometry2 = new THREE.BufferGeometry();
		let pOsitions2 = [];
		pOsitions2.push(0, 0, 0.0);
		pOsitions2.push(1, 0, 0.0);
		this.lineGeometry2.addAttribute('position', new THREE.Float32BufferAttribute(pOsitions2, 3));
		let lineMeshX = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('red') }));
		lineMeshX.scale.x = 22;
		let lineMeshY = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('black') }));
		lineMeshY.scale.x = 1;
		lineMeshY.rotation.z = Math.PI/2;
		let lineMeshZ = new THREE.Line(this.lineGeometry2, new THREE.LineBasicMaterial({ color: new THREE.Color('blue') }));
		lineMeshZ.scale.x = 1;
		lineMeshZ.rotation.y = -Math.PI/2;
		this.diagram = new THREE.Object3D();
		this.diagram2 = new THREE.Object3D();
		this.diagram.add(this.diagram2);
		this.diagram2.add(this.camera);
		
		// init in view from diagonally below
		this.diagram.rotation.x = -0.5;
		this.diagram2.rotation.y = -0.25;
		
		
		// RENDERER SETUP
		
		this.canvas = document.getElementById(canvasName);
		this.renderer = new THREE.WebGLRenderer( {canvas: this.canvas, antialias: true } );
		this.renderer.shadowMap.enabled = true;
		this.renderer.shadowMap.type = THREE.PCFSoftShadowMap;
		
		
		// SCENE SETUP
		
		this.scene = new THREE.Scene();
		this.scene.background = new THREE.Color(0x87ceeb);
		this.scene.add(lineMeshX); this.scene.add(lineMeshY); this.scene.add(lineMeshZ);
		this.scene.add(this.diagram);
		///this.scene.add( this.dirLight );
		this.scene.add( hemiLight );
		//this.scene.add(new THREE.CameraHelper(this.dirLight.shadow.camera));
		
		
		// EVENT HANDLING
		
		this.oldX = 0;
		this.oldY = 0;
		this.mousedown = false;
		
		let mouseDown = function(event){
			this.oldX = event.clientX;
			this.oldY = event.clientY;
			this.mousedown = true;
			let d = new Date(); this.mouseDownTime = d.getTime();
		}.bind(this);
		
		let mouseMove = function(event){
			if(this.mousedown){
				this.diagram.rotation.x -= (event.clientX - this.oldX)/100.0;
				this.diagram2.rotation.y -= (event.clientY - this.oldY)/100.0;
				this.oldX = event.clientX;
				this.oldY = event.clientY;
			}
		}.bind(this);
		
		let mouseCancel = function(event) {
			this.mousedown = false;
			let d = new Date();
			if((d.getTime() - this.mouseDownTime) < 200){
				this.pauseCallback();
			}
		}.bind(this);
		
		let mouseWheel = function(event) {
			var scale = Math.pow(1.1, Math.sign(event.deltaY));
			this.camera.position.z /= scale;
			this.cameraOrtho.zoom *= scale;
			/*this.camera.left *= scale;
			this.camera.right *= scale;
			this.camera.top *= scale;
			this.camera.bottom *= scale;*/
			this.camera.updateProjectionMatrix();
			this.cameraOrtho.updateProjectionMatrix();
			
		}.bind(this);
		
		this.no_of_fingers = 0;
		this.zoom = false;
		this.zoom_pinch_done = false;
		this.touchZoomDistanceEnd = 0;
		this.touchZoomDistanceStart = 0;
		
		let onDocumentTouchStart = function(event){
			
			event.preventDefault();
			
			this.no_of_fingers = event.touches.length;
			
			if(this.no_of_fingers == 1){
				this.zoom = false;
				//event = event.changedTouches[0];
				//this.handleStart(event);
				mouseDown(event.changedTouches[0]);
				let d = new Date(); this.mouseDownTime = d.getTime();
			}else if(this.no_of_fingers == 2){
				this.zoom = true;
				//alert("zoom true");
				var dx = event.touches[ 0 ].pageX - event.touches[ 1 ].pageX;
				var dy = event.touches[ 0 ].pageY - event.touches[ 1 ].pageY;
				
				this.touchZoomDistanceEnd = this.touchZoomDistanceStart = Math.sqrt( dx * dx + dy * dy );
				
			}else{
				this.zoom_pinch_done = true;
			}
			
		}.bind(this);
		
		let onDocumentTouchMove = function(event){
			
			event.preventDefault();
			
			if(this.zoom){
				var dx = event.touches[ 0 ].clientX - event.touches[ 1 ].clientX;
				var dy = event.touches[ 0 ].clientY - event.touches[ 1 ].clientY;
				this.touchZoomDistanceEnd = Math.sqrt( dx * dx + dy * dy );
				
				//alert("zooming with factor " + (this.touchZoomDistanceEnd/this.touchZoomDistanceStart));
				this.camera.position.z /= this.touchZoomDistanceEnd/this.touchZoomDistanceStart;
				this.cameraOrtho.zoom *= this.touchZoomDistanceEnd/this.touchZoomDistanceStart;
				this.cameraOrtho.updateProjectionMatrix();
				this.camera.updateProjectionMatrix();
				
				this.touchZoomDistanceStart = this.touchZoomDistanceEnd;
			}else if (!this.zoom_pinch_done){
				mouseMove(event.changedTouches[0]);
			}
		}.bind(this);
		
		let onDocumentTouchEnd = function(event){
			
			event.preventDefault();
			
			this.no_of_fingers = event.touches.length;
			if(this.no_of_fingers == 0){
				if(!this.zoom_pinch_done){
					this.mouseCancel(event.changedTouches[0]);
					let d = new Date();
					if((d.getTime() - this.mouseDownTime) < 200){
						this.pauseCallback();
					}
				}
				this.zoom_pinch_done = false;
			}else if(this.no_of_fingers == 1){
				//finish zooming.
				this.zoom_pinch_done = true;
				this.zoom = false;
			}
			
		}.bind(this);
		
		
		this.canvas.addEventListener( 'mousedown', function(event){event.preventDefault();mouseDown(event);}, false );
		this.canvas.addEventListener( 'mousemove', function(event){event.preventDefault();mouseMove(event);}, false );
		
		this.canvas.addEventListener( 'mouseup', function(event){event.preventDefault();mouseCancel(event);}, false );
		this.canvas.addEventListener( 'mouseleave', function(event){event.preventDefault();mouseCancel(event);}, false );
		this.canvas.addEventListener( 'wheel', function(event){event.preventDefault();mouseWheel(event);}, false );
		
		
		this.canvas.addEventListener( 'touchstart', function(event){event.preventDefault();onDocumentTouchStart(event);}, false );
		this.canvas.addEventListener( 'touchmove', function(event){event.preventDefault();onDocumentTouchMove(event);}, false );
		this.canvas.addEventListener( 'touchend', function(event){event.preventDefault();onDocumentTouchEnd(event);}, false );
		this.canvas.addEventListener( 'touchcancel', function(event){event.preventDefault();onDocumentTouchEnd(event);}, false );
		
		this.spaceBarPressed = false;
		this.fullscreen = false;
		document.addEventListener("keydown", function(event){
			if(event.keyCode == 32){
				event.preventDefault();
				if(!this.spaceBarPressed){
					let d = new Date(); this.spaceBarDownTime = d.getTime();
					//this.beginFF();
					this.spaceBarPressed = true;
				}
			}else if (event.keyCode == 70){
				event.preventDefault();
				if(this.fullscreen){
					document.exitFullscreen();
				}else{
					this.canvas.requestFullscreen();
					
				}
			}else if(event.keyCode == 83){
				event.preventDefault();
				if(!this.sPressed){
					this.beginSpeed();
					this.sPressed = true;
				}
			}
		}.bind(this), false);
		
		document.addEventListener('fullscreenchange', function(event){
			if(this.fullscreen){
				this.canvas.width = 1200;
		    	this.canvas.height = 900;
		    	this.camera.aspect = 1200/900;
		    	this.camera.updateProjectionMatrix();
		    	this.renderer.setSize(1200, 900);
		    	this.fullscreen = false;
		    }else{
		    	this.canvas.width = screen.width;
        		this.canvas.height = screen.height;
        		this.camera.aspect = screen.width/screen.height;
        		this.camera.updateProjectionMatrix();
        		this.renderer.setSize(screen.width, screen.height);
				this.fullscreen = true;
		    }
		}.bind(this), false);
		
		document.addEventListener("keyup", function(event){
			if(event.keyCode == 32){
				event.preventDefault();
				let d = new Date();
				if((d.getTime() - this.spaceBarDownTime) < 200){
					this.pauseCallback();
				}
				this.endFF();
				this.spaceBarPressed = false;
			}else if(event.keyCode == 83){
				event.preventDefault();
				this.endSpeed();
				this.sPressed = false;
			}
		}.bind(this), false);
	}
	
	render(){
		let d = new Date();
		if(this.spaceBarPressed && (d.getTime() - this.spaceBarDownTime) > 200) this.beginFF();
		this.renderer.render( this.scene, this.cameraIsOrtho?this.cameraOrtho:this.camera );
	}
	
}
