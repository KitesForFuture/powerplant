"use strict";

class Kite extends RigidBody{
	
	setDimensions(x, y, z){
		this.dimensions.x = x;
		this.dimensions.y = y;
		this.dimensions.z = z;
		
		this.updateDimensionsOfParts();
	}
	
	updateDimensionsAndPositionsOfParts(){
		this.rudderData.StabilizerDimensions.x = this.rudderData.stabilizerDepth*this.dimensions.y*0.5
		this.stabilizerLeft.setDimensions(this.rudderData.StabilizerDimensions.x, this.rudderData.StabilizerDimensions.y, this.rudderData.StabilizerDimensions.z);
		this.stabilizerLeft.position.x = -this.rudderData.distanceFromCG - this.rudderData.StabilizerDimensions.x * 0.5;
		this.stabilizerLeft.position.z = 0;//this.rudderData.StabilizerDimensions.y*0.5;
		this.stabilizerLeft.position.y = 0;//this.dimensions.y*0.5;
		this.rudderData.distanceFromCG = this.rudderData.distanceFromCGInPercent*this.dimensions.y*0.5;
		this.rudder.position.x = -this.rudderData.distanceFromCG;
		this.rudder.position.z = 0;//this.rudderData.dimensions.y*0.5;
		
		this.reflexMeshLeft.setDimensions(this.reflexData.dimensions.x, (1-this.elevonData.dimensions.y)*this.dimensions.y*0.5, this.reflexData.dimensions.z);
		this.reflexMeshLeft.position.x = - this.reflexData.dimensions.x*0.5;
		this.leftReflex.position.x = - this.dimensions.x * ( 1 - this.center_of_gravity_from_front );// + 0.2;
		//this.leftReflex.rotation.y = Math.PI * 5/180;
		this.reflexMeshLeft.position.y = (1-this.elevonData.dimensions.y)*this.dimensions.y*0.25;
		
		this.reflexMeshRight.setDimensions(this.reflexData.dimensions.x, (1-this.elevonData.dimensions.y)*this.dimensions.y*0.5, this.reflexData.dimensions.z);
		this.reflexMeshRight.position.x = - this.reflexData.dimensions.x*0.5;
		this.rightReflex.position.x = - this.dimensions.x * ( 1 - this.center_of_gravity_from_front );// + 0.2;
		//this.rightReflex.rotation.y = Math.PI * 5/180;
		this.reflexMeshRight.position.y = - (1-this.elevonData.dimensions.y)*this.dimensions.y*0.25;
		
		this.elevonMeshLeft.setDimensions(this.elevonData.dimensions.x, this.elevonData.dimensions.y*this.dimensions.y*0.5, this.elevonData.dimensions.z);
		this.elevonMeshRight.setDimensions(this.elevonData.dimensions.x, this.elevonData.dimensions.y*this.dimensions.y*0.5, this.elevonData.dimensions.z);
		this.elevonMeshLeft.position.x = - this.elevonData.dimensions.x*0.5;
		this.leftElevon.position.x = - this.dimensions.x * ( 1 - this.center_of_gravity_from_front );// + 0.2;
		
		this.elevonMeshRight.position.x = - this.elevonData.dimensions.x*0.5;
		this.rightElevon.position.x = - this.dimensions.x*(1 - this.center_of_gravity_from_front);// + 0.2;
		
		this.elevonMeshLeft.position.y = (1-0.5*this.elevonData.dimensions.y)*this.dimensions.y*0.5;
		this.elevonMeshRight.position.y = - (1-0.5*this.elevonData.dimensions.y)*this.dimensions.y*0.5;
		
		
		this.setTannenbaumLengthAndBridleLength(this.tannenbaum_length);
		
		this.simpleWing1.setDimensions(this.dimensions.x, this.dimensions.y*0.5, this.dimensions.z);
		this.simpleWing2.setDimensions(this.dimensions.x, this.dimensions.y*0.5, this.dimensions.z);
		
		this.simpleWing1.position.x = - this.dimensions.x*(0.5-this.center_of_gravity_from_front);
		this.simpleWing2.position.x = - this.dimensions.x*(0.5-this.center_of_gravity_from_front);
		
		this.simpleWing1.position.y = + this.dimensions.y*0.25;
		this.simpleWing2.position.y = - this.dimensions.y*0.25;
		
		
		this.leftPropeller.position.x = this.dimensions.x*this.center_of_gravity_from_front + 0.05;// + this.propeller_radius;
		this.rightPropeller.position.x = this.dimensions.x*this.center_of_gravity_from_front + 0.05;// + this.propeller_radius;
		this.leftPropeller.position.y = this.dimensions.y*0.5 * ((1-this.elevonData.dimensions.y) + this.propDistanceFromCenter*this.elevonData.dimensions.y);
		this.rightPropeller.position.y = -this.dimensions.y*0.5 * ((1-this.elevonData.dimensions.y) + this.propDistanceFromCenter*this.elevonData.dimensions.y);
		
		this.calculateAndSetAngularInertiaAndMass();
		
		//this.setPropellerDistance(this.elevonData.distanceFromCenter);
		
		this.cgIndicator.setScale(this.dimensions.y + 0.01);
		
		this.calculateAndSetAngularInertiaAndMass();
	}
	
	reset(){
		//console.log("RigidBody.reset()");
		this.positionR.set(2, 0, 2);
		
		this.velocity = new THREE.Vector3(0, 0, 0);
		
		this.rotation_matrix.set(
			0,   0, 1,
			0,   1, 0,
			-1,   0, 0
		);
		
		this.angular_velocity = new THREE.Vector3(0, 0, 0);
	}
	
	constructor(){
		
		super();
		//this.positionR.set(2, 0, 2);//this.positionR.set(40, 0, 82);//
		this.reset();
		this.dimensions = new THREE.Vector3(0.2, 2, 0.03);
		
		this.center_of_gravity_from_front = 0.25;//30;//0.33;//0.33;//fraction of the full chord length
		this.bridle_length = 0.2;//-0.05;//in meters, with respect to the wing join center
		this.tannenbaum_length = 0.05;
		this.height_of_c_g = 0;//-0.08;// with respect to the c_g of the wing geometry.
		this.elevonData = new Object();
		this.elevonData.dimensions = new THREE.Vector3(0.08, 0.5, 0.03);
		this.propDistanceFromCenter = 0.5;
		this.rudderData = new Object();
		this.rudderData.dimensions = new THREE.Vector3(0.1, 0.4, 0.01);
		this.rudderData.stabilizerDepth = 0.2;
		this.rudderData.StabilizerDimensions = new THREE.Vector3(0.25, 0.2, 0.01);
		this.rudderData.distanceFromCGInPercent = 0.15;
		this.rudderData.distanceFromCG = 0.15;
		this.reflexData = new Object();
		this.reflexData.dimensions = new THREE.Vector3(0.10, 0.5, 0.03);
		this.maxPropellerThrust = 26;
		this.propeller_radius = 0.18;
		this.bridle_length_percentage = 0.2;
		// STABILIZER
		this.stabilizerLeft = new FlatPlate(this.rudderData.StabilizerDimensions.x, this.rudderData.StabilizerDimensions.y, this.rudderData.StabilizerDimensions.z, this , 'cornflowerblue', false, false);
		this.stabilizerLeft.rotation.x = 0.5*Math.PI;
		
		/*
		this.stabilizerRight = new FlatPlate(this.rudderData.StabilizerDimensions.x, this.rudderData.StabilizerDimensions.y, this.rudderData.StabilizerDimensions.z, this , 'blue', false);
		this.stabilizerRight.rotation.x = 0.5*Math.PI;
		this.stabilizerRight.position.x = -this.rudderData.distanceFromCG + this.rudderData.dimensions.x*0.5 + this.rudderData.StabilizerDimensions.x * 0.5;
		this.stabilizerRight.position.z = this.rudderData.StabilizerDimensions.y*0.5;
		this.stabilizerRight.position.y = -this.dimensions.y*0.5;
		*/
		// RUDDER
		this.rudder = new FlatPlate(this.rudderData.dimensions.x, this.rudderData.dimensions.y, this.rudderData.dimensions.z, this , 'cornflowerblue', false, false);
		this.rudder.rotation.x = 0.5*Math.PI;
		
		
		// LEFT REFLEX
		this.reflexMeshLeft = new FlatPlate(this.reflexData.dimensions.x, this.reflexData.dimensions.y, this.reflexData.dimensions.z, this, 'gold', true, true);
		
		this.leftReflex = new THREE.Object3D();
		this.leftReflex.add (this.reflexMeshLeft);
		
		
		
		// RIGHT REFLEX
		this.reflexMeshRight = new FlatPlate(this.reflexData.dimensions.x, this.reflexData.dimensions.y, this.reflexData.dimensions.z, this, 'gold', true, true);
		
		this.rightReflex = new THREE.Object3D();
		this.rightReflex.add (this.reflexMeshRight);
		
		
		// LEFT ELEVON
		this.elevonMeshLeft = new FlatPlate(this.elevonData.dimensions.x, this.elevonData.dimensions.y, this.elevonData.dimensions.z, this, 'coral', true, true);
		
		this.leftElevon = new THREE.Object3D();
		this.leftElevon.add ( this.elevonMeshLeft );
		
		this.leftElevon.rotation.y = 0;//Math.PI * 0.1;
		
		// RIGHT ELEVON
		this.elevonMeshRight = new FlatPlate(this.elevonData.dimensions.x, this.elevonData.dimensions.y, this.elevonData.dimensions.z, this, 'coral', true, true);
		
		this.rightElevon = new THREE.Object3D();
		this.rightElevon.add(this.elevonMeshRight);
		
		this.rightElevon.rotation.y = 0;//Math.PI * 0.1;
		
		// TANNENBAUM
		this.tannenbaumLeft = new THREE.Mesh(new THREE.BoxBufferGeometry(0.03, 0.001, 1), new THREE.MeshPhongMaterial( { color: new THREE.Color('cornflowerblue') } ) );
		this.tannenbaumRight = new THREE.Mesh(new THREE.BoxBufferGeometry(0.03, 0.001, 1), new THREE.MeshPhongMaterial( { color: new THREE.Color('cornflowerblue') } ) );
		
		// WINGS
		this.simpleWing1 = new FlatPlate(this.dimensions.x, this.dimensions.y*0.5, this.dimensions.z, this, 'lightsteelblue', true, false);
		this.simpleWing2 = new FlatPlate(this.dimensions.x, this.dimensions.y*0.5, this.dimensions.z, this, 'lightsteelblue', true, false);
		
		// PROPELLERS
		
		this.leftPropeller = new Propeller(this.propeller_radius, this, this.maxPropellerThrust);
		this.leftPropeller.rotation.z = -Math.PI*0.5;
		
		this.leftPropeller.fold();
		
		this.rightPropeller = new Propeller(this.propeller_radius, this, this.maxPropellerThrust);
		this.rightPropeller.rotation.z = -Math.PI*0.5;
		
		this.cgIndicator = new CGIndicator(this);
		
		this.updateDimensionsAndPositionsOfParts();
		
		this.lineTensionTorqueVis = new VectorVis(new THREE.Color('red'));
		this.lineTensionTorqueVis2 = new VectorVis(new THREE.Color('red'));
		this.massVis = new VectorVis(new THREE.Color('black'));
		
		this.tangentCoordsVisNorth = new VectorVis(new THREE.Color('black'));
		this.tangentCoordsVisLeft = new VectorVis(new THREE.Color('black'));
		this.targetAngleVis = new VectorVis(new THREE.Color('magenta'));
		this.diveAngleVis = new VectorVis(new THREE.Color('red'));
		//this.Wing1TorqueVis = new VectorVis(new THREE.Color('blue'));
		//this.Wing2TorqueVis = new VectorVis(new THREE.Color('blue'));
		//this.StabilizerTorqueVis = new VectorVis(new THREE.Color('yellow'));
		// ASSEMBLY OF VISIBLE PARTS
		let visibleKiteParts = [
			this.tannenbaumLeft,
			this.tannenbaumRight,
			this.simpleWing1,
			this.simpleWing2,
			//this.wingDrag,
			this.leftElevon,
			this.rightElevon,
			this.leftReflex,
			this.rightReflex,
			this.leftPropeller,
			this.rightPropeller,
			this.stabilizerLeft,
			//this.stabilizerRight,
			//this.rudder
			this.lineTensionTorqueVis,
			this.lineTensionTorqueVis2,
			this.massVis,
			this.targetAngleVis,
			this.diveAngleVis,
			this.tangentCoordsVisNorth,
			this.tangentCoordsVisLeft,
			//this.Wing1TorqueVis,
			//this.Wing2TorqueVis,
			//this.StabilizerTorqueVis
			this.cgIndicator,
		];
		
		this.kiteMeshes = new THREE.Object3D();
		this.kiteMeshes.position.z -= this.height_of_c_g;
		for (let visiblePart of visibleKiteParts){
			this.kiteMeshes.add(visiblePart);
		}
		
		this.add(this.kiteMeshes);
		
		this.simpleWing1.name = "wing1";
		this.simpleWing2.name = "wing2";
		this.elevonMeshLeft.name = "elevon mesh left";
		this.elevonMeshRight.name = "elevon mesh right";
		this.reflexMeshLeft.name = "reflex mesh left";
		this.reflexMeshRight.name = "reflex mesh right";
		this.stabilizerLeft.name = "stabilizer";
		this.leftPropeller.name = "left prop";
		this.rightPropeller.name = "right prop";
		
		// AERODYNAMIC PARTS
		this.aerodynamicKiteParts = [
			this.simpleWing1,
			this.simpleWing2,
			this.elevonMeshLeft,
			this.elevonMeshRight,
			this.reflexMeshLeft,
			this.reflexMeshRight,
			//this.rudder,
			this.stabilizerLeft,
			//this.stabilizerRight,
			//this.wingDrag,
			this.leftPropeller,
			this.rightPropeller
		];
		
		for (let part of this.aerodynamicKiteParts){
			if(part.torqueVis)
				this.kiteMeshes.add(part.torqueVis);
		}
	}
	
	calculateAndSetAngularInertiaAndMass(){
		var getInertia = function(mass, x, y, z){
			var vec = new THREE.Vector3();
			vec.set(y*y+z*z, x*x+z*z, x*x+y*y);
			return vec.multiplyScalar(1/12);
		}
		
		var spar_mass = 0.160* this.dimensions.y*this.dimensions.z/0.06;
		var spar_inertia = getInertia(spar_mass, 0.01, this.dimensions.y, 0.5*this.dimensions.z);
		
		var xps_mass = 0.200 * this.dimensions.x*this.dimensions.y*this.dimensions.z/0.012;;
		var xps_inertia = getInertia(xps_mass, this.dimensions.x, this.dimensions.y, this.dimensions.z);
		
		var cfk_mass = 0.050 * this.dimensions.x*this.dimensions.y*this.dimensions.z/0.012;
		var cfk_inertia = getInertia(cfk_mass, this.dimensions.x, this.dimensions.y, this.dimensions.z);
		
		var battery_mass = 0.180;
		var m_d_d_battery = battery_mass * 0.14*0.14; // mass * distance**2
		var battery_inertia = new THREE.Vector3();
		battery_inertia.set(0, m_d_d_battery, m_d_d_battery);
		
		var motors_mass = 2*0.100;
		var dist_motors = this.propDistanceFromCenter;
		var motors_inertia = new THREE.Vector3();
		motors_inertia.set(motors_mass*dist_motors*dist_motors, motors_mass*0.14*0.14, motors_mass*(dist_motors*dist_motors + 0.14*0.14));
		
		var servo_mass = 2*0.020;
		var servo_x = 0.15;
		var servo_y = this.propDistanceFromCenter;
		var servo_inertia = new THREE.Vector3();
		servo_inertia.set(servo_mass*servo_y*servo_y, servo_mass*servo_x*servo_x, servo_mass*(servo_y*servo_y + servo_x*servo_x));
		
		this.mass = spar_mass + xps_mass + cfk_mass + battery_mass + motors_mass + servo_mass;
		
		this.angular_inertia = spar_inertia.add(xps_inertia.add(cfk_inertia.add(battery_inertia.add(motors_inertia.add(servo_inertia)))));
	}
	
	setReflex(reflex_angle_in_radians){
		this.leftReflex.rotation.y = -Math.PI*0.5;//reflex_angle_in_radians;
		this.rightReflex.rotation.y = -Math.PI*0.5;//reflex_angle_in_radians;
	}
	
	setTannenbaumLengthAndBridleLength(tannenbaum_length){
		
		this.tannenbaum_length = tannenbaum_length;		//z-axis goes other direction than tannenbaum
		this.bridle_length = this.bridle_length_percentage*this.dimensions.y*0.5;		//z-axis goes other direction than tannenbaum
		
		this.tannenbaumLeft.position.z = - this.tannenbaum_length*0.5;
		this.tannenbaumLeft.position.y = 0.25*this.dimensions.y;
		this.tannenbaumLeft.scale.z = tannenbaum_length;
		
		this.tannenbaumRight.position.z = - this.tannenbaum_length*0.5;
		this.tannenbaumRight.position.y = -0.25*this.dimensions.y;
		this.tannenbaumRight.scale.z = tannenbaum_length;
		
	}
	
	setPropellerDistance(prop_dist){
		this.elevonData.distanceFromCenter = prop_dist;
		
		//this.elevonMeshLeft.position.y = this.dimensions.y*0.5 * this.elevonData.distanceFromCenter;
		//this.elevonMeshRight.position.y = - this.dimensions.y*0.5 * this.elevonData.distanceFromCenter;
		
		this.leftPropeller.position.y = this.dimensions.y*0.5 * this.elevonData.distanceFromCenter;
		this.rightPropeller.position.y = -this.dimensions.y*0.5 * this.elevonData.distanceFromCenter;
		
		this.calculateAndSetAngularInertiaAndMass();
	}
	
	getRelativePositionOfLineKnotInKiteCoordinates(){
		this.kiteMeshes.updateWorldMatrix(true, false);
		var y_axis = new THREE.Vector3(this.matrix.elements[4], this.matrix.elements[5], this.matrix.elements[6]); // it's norm = 1
		var pos_of_kite = new THREE.Vector3(this.kiteMeshes.matrixWorld.elements[12], this.kiteMeshes.matrixWorld.elements[13], this.kiteMeshes.matrixWorld.elements[14]);
		
		var z_axis = new THREE.Vector3(this.matrix.elements[8], this.matrix.elements[9], this.matrix.elements[10]);
		// PROJECT (kite_position - z-axis * tannenbaum_length) ONTO COMPLEMENT OF y-axis ...
		// ... to get ...
		// ... direction between bridle point and midpoint between the tannenbaums.
		var lineKnotDir = projectToComplementOfNormedVector(pos_of_kite.add(z_axis.multiplyScalar(this.tannenbaum_length)), y_axis);
		lineKnotDir.normalize().multiplyScalar(-this.bridle_length);
		
		return (new THREE.Vector3(0, 0, -this.tannenbaum_length)).add(this.world2kite(lineKnotDir));
	}
	
	
	getPositionOfLineKnot(){
		this.kiteMeshes.updateWorldMatrix(true, false);
		var y_axis = new THREE.Vector3(this.matrix.elements[4], this.matrix.elements[5], this.matrix.elements[6]); // it's norm = 1
		var pos_of_kite = new THREE.Vector3(this.kiteMeshes.matrixWorld.elements[12], this.kiteMeshes.matrixWorld.elements[13], this.kiteMeshes.matrixWorld.elements[14]);
		var z_axis = new THREE.Vector3(this.matrix.elements[8], this.matrix.elements[9], this.matrix.elements[10]);
		var lineKnotDir = projectToComplementOfNormedVector(pos_of_kite.clone().add(z_axis.clone().multiplyScalar(-this.tannenbaum_length)), y_axis);
		lineKnotDir.normalize().multiplyScalar(this.bridle_length);
		
		var lineKnot = pos_of_kite.add(z_axis.clone().multiplyScalar(-this.tannenbaum_length)).sub(lineKnotDir);
		
		return lineKnot;
	}
	
	update(wind, line_tension, timestep_in_s){
		
		let leftForceVec = this.leftPropeller.getForceAndCentreOfPressureInKiteCoords().force;
		let rightForceVec = this.rightPropeller.getForceAndCentreOfPressureInKiteCoords().force;
		//console.log("Fr = " + rightForceVec.length() + ", Fl = " + leftForceVec.length());
		if(leftForceVec.length() > 0.001 && rightForceVec.length() > 0.001){
			this.elevonMeshLeft.setAdditionalWind(leftForceVec.multiplyScalar(-2/Math.sqrt(leftForceVec.length())));
			this.elevonMeshRight.setAdditionalWind(rightForceVec.multiplyScalar(-2/Math.sqrt(rightForceVec.length())));
		}else{
			this.elevonMeshLeft.setAdditionalWind(new THREE.Vector3(0,0,0));
			this.elevonMeshRight.setAdditionalWind(new THREE.Vector3(0,0,0));
		}
		
		let wind_vector = wind.getWindVector();
		let force = new THREE.Vector3();
		let torque = new THREE.Vector3(0, 0, 0);
		// adding all forces and torques
		for (let kitePart of this.aerodynamicKiteParts){
			let forceData = kitePart.getForceAndCentreOfPressureInKiteCoords(wind_vector);
			//console.log(kitePart);
			force.add(forceData.force);
			torque.add(forceData.pressure_centre.clone().cross(this.world2kite(forceData.force.clone())));
			if(kitePart.torqueVis) kitePart.torqueVis.set(forceData.pressure_centre, this.world2kite(forceData.force));
		}
		
		//console.log("---");
		//console.log(wingForceData.force);
		// add line tension to force and torque
		let direction_of_line_tension_force = this.getPositionOfLineKnot().normalize();
		//console.log(direction_of_line_tension_force);
		let line_force_dir_in_kite_coords = this.world2kite(direction_of_line_tension_force.clone());
		//console.log(line_force_dir_in_kite_coords);
		force.add(direction_of_line_tension_force.multiplyScalar(-line_tension));
		torque.add(line_force_dir_in_kite_coords.clone().cross(this.getRelativePositionOfLineKnotInKiteCoordinates()).multiplyScalar(line_tension));
		this.lineTensionTorqueVis.set(this.getRelativePositionOfLineKnotInKiteCoordinates().add(new THREE.Vector3(0.001, 0, 0)), line_force_dir_in_kite_coords.multiplyScalar(-line_tension));
		this.lineTensionTorqueVis2.set(this.getRelativePositionOfLineKnotInKiteCoordinates().add(new THREE.Vector3(-0.001, 0, 0)), line_force_dir_in_kite_coords);
		this.massVis.set(new THREE.Vector3(0, 0, 0), this.world2kite(new THREE.Vector3(-10*this.mass, 0, 0)));
		// add gravity to force
		let gravity = new THREE.Vector3(-9.81, 0, 0);
		force.add(new THREE.Vector3(-10*this.mass, 0, 0));
		
		super.update(force, torque, timestep_in_s);
	}
}
