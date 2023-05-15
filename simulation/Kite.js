"use strict";

class Kite extends RigidBody{
	
	constructor(){ super();
		
		this.positionR.set(2, 0, 2);//this.positionR.set(40, 0, 82);//
		this.dimensions = new THREE.Vector3(0.2, 2, 0.03);
		
		this.center_of_gravity_from_front = 0.25;//30;//0.33;//0.33;//fraction of the full chord length
		this.bridle_length = 0.2;//-0.05;//in meters, with respect to the wing join center
		this.tannenbaum_length = 0.05;
		this.height_of_c_g = 0;//-0.08;// with respect to the c_g of the wing geometry.
		this.elevonData = new Object();
		this.elevonData.dimensions = new THREE.Vector3(0.08, 0.5, 0.005);
		this.elevonData.distanceFromCenter = 0.75;
		this.rudderData = new Object();
		this.rudderData.dimensions = new THREE.Vector3(0.1, 0.4, 0.005);
		this.rudderData.StabilizerDimensions = new THREE.Vector3(0.25, 0.2, 0.005);
		this.rudderData.distanceFromCG = 0.45;
		this.reflexData = new Object();
		this.reflexData.dimensions = new THREE.Vector3(0.10, 0.5, 0.005);
		this.maxPropellerThrust = 8;
		
		// STABILIZER
		this.stabilizerLeft = new FlatPlate(this.rudderData.StabilizerDimensions.x, this.rudderData.StabilizerDimensions.y, this.rudderData.StabilizerDimensions.z, this , 'blue', false);
		this.stabilizerLeft.rotation.x = 0.5*Math.PI;
		this.stabilizerLeft.position.x = -this.rudderData.distanceFromCG + this.rudderData.dimensions.x*0.5 + this.rudderData.StabilizerDimensions.x * 0.5;
		this.stabilizerLeft.position.z = 0;//this.rudderData.StabilizerDimensions.y*0.5;
		this.stabilizerLeft.position.y = 0;//this.dimensions.y*0.5;
		/*
		this.stabilizerRight = new FlatPlate(this.rudderData.StabilizerDimensions.x, this.rudderData.StabilizerDimensions.y, this.rudderData.StabilizerDimensions.z, this , 'blue', false);
		this.stabilizerRight.rotation.x = 0.5*Math.PI;
		this.stabilizerRight.position.x = -this.rudderData.distanceFromCG + this.rudderData.dimensions.x*0.5 + this.rudderData.StabilizerDimensions.x * 0.5;
		this.stabilizerRight.position.z = this.rudderData.StabilizerDimensions.y*0.5;
		this.stabilizerRight.position.y = -this.dimensions.y*0.5;
		*/
		// RUDDER
		this.rudder = new FlatPlate(this.rudderData.dimensions.x, this.rudderData.dimensions.y, this.rudderData.dimensions.z, this , 'magenta', false);
		this.rudder.rotation.x = 0.5*Math.PI;
		this.rudder.position.x = -this.rudderData.distanceFromCG;
		this.rudder.position.z = 0;//this.rudderData.dimensions.y*0.5;
		
		// LEFT REFLEX
		this.reflexMeshLeft = new FlatPlate(this.reflexData.dimensions.x, this.reflexData.dimensions.y, this.reflexData.dimensions.z, this, 'teal', false);
		this.reflexMeshLeft.position.x -= this.reflexData.dimensions.x*0.5;
		this.leftReflex = new THREE.Object3D();
		this.leftReflex.add (this.reflexMeshLeft);
		this.leftReflex.position.x -= this.dimensions.x * ( 1 - this.center_of_gravity_from_front );// + 0.2;
		this.leftReflex.rotation.y = Math.PI * 15/180;
		this.reflexMeshLeft.position.y = 0.25;//this.dimensions.y * 0.25;
		
		// RIGHT REFLEX
		this.reflexMeshRight = new FlatPlate(this.reflexData.dimensions.x, this.reflexData.dimensions.y, this.reflexData.dimensions.z, this, 'teal', false);
		this.reflexMeshRight.position.x -= this.reflexData.dimensions.x*0.5;
		this.rightReflex = new THREE.Object3D();
		this.rightReflex.add (this.reflexMeshRight);
		this.rightReflex.position.x -= this.dimensions.x * ( 1 - this.center_of_gravity_from_front );// + 0.2;
		this.rightReflex.rotation.y = Math.PI * 15/180;
		this.reflexMeshRight.position.y = - 0.25;//this.dimensions.y * 0.25;
		
		// LEFT ELEVON
		this.elevonMeshLeft = new FlatPlate(this.elevonData.dimensions.x, this.elevonData.dimensions.y, this.elevonData.dimensions.z, this, 'magenta', false);
		this.elevonMeshLeft.position.x -= this.elevonData.dimensions.x*0.5;
		this.leftElevon = new THREE.Object3D();
		this.leftElevon.add ( this.elevonMeshLeft );
		this.leftElevon.position.x -= this.dimensions.x * ( 1 - this.center_of_gravity_from_front );// + 0.2;
		this.leftElevon.rotation.y = 0;//Math.PI * 0.1;
		
		// RIGHT ELEVON
		this.elevonMeshRight = new FlatPlate(this.elevonData.dimensions.x, this.elevonData.dimensions.y, this.elevonData.dimensions.z, this, 'magenta', false);
		this.elevonMeshRight.position.x -= this.elevonData.dimensions.x*0.5;
		this.rightElevon = new THREE.Object3D();
		this.rightElevon.add(this.elevonMeshRight);
		this.rightElevon.position.x -= this.dimensions.x*(1 - this.center_of_gravity_from_front);// + 0.2;
		this.rightElevon.rotation.y = 0;//Math.PI * 0.1;
		
		// TANNENBAUM
		this.tannenbaumLeft = new THREE.Mesh(new THREE.BoxBufferGeometry(0.03, 0.001, 1), new THREE.MeshBasicMaterial( { color: new THREE.Color('blue') } ) );
		this.tannenbaumRight = new THREE.Mesh(new THREE.BoxBufferGeometry(0.03, 0.001, 1), new THREE.MeshBasicMaterial( { color: new THREE.Color('blue') } ) );
		this.setTannenbaumLengthAndBridleLength(this.tannenbaum_length, this.bridle_length);
		
		// WINGS
		this.simpleWing1 = new FlatPlate(this.dimensions.x, this.dimensions.y*0.5, this.dimensions.z, this, 'white', true);
		this.simpleWing2 = new FlatPlate(this.dimensions.x, this.dimensions.y*0.5, this.dimensions.z, this, 'white', true);
		
		this.simpleWing1.position.x -= this.dimensions.x*(0.5-this.center_of_gravity_from_front);
		this.simpleWing2.position.x -= this.dimensions.x*(0.5-this.center_of_gravity_from_front);
		
		this.simpleWing1.position.y = + this.dimensions.y*0.25;
		this.simpleWing2.position.y = - this.dimensions.y*0.25;
		
		
		
		//this.wingDrag = new FlatPlate(0.0025, this.dimensions.y, this.dimensions.z, this, 'white', false);
		//this.wingDrag.position.x = this.dimensions.x * 0.25;
		//this.wingDrag.rotation.y = 0.5*Math.PI;
		
		// PROPELLERS
		
		var propeller_radius = 0.1;
		
		this.leftPropeller = new Propeller(propeller_radius, this, this.maxPropellerThrust);
		this.leftPropeller.rotation.z = -Math.PI*0.5;
		this.leftPropeller.position.x = this.dimensions.x*this.center_of_gravity_from_front + propeller_radius;
		this.leftPropeller.fold();
		
		this.rightPropeller = new Propeller(propeller_radius, this, this.maxPropellerThrust);
		this.rightPropeller.rotation.z = -Math.PI*0.5;
		this.rightPropeller.position.x = this.dimensions.x*this.center_of_gravity_from_front + propeller_radius;
		
		this.setPropellerDistance(this.elevonData.distanceFromCenter);
		
		this.lineTensionTorqueVis = new VectorVis(new THREE.Color('red'));
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
			//this.Wing1TorqueVis,
			//this.Wing2TorqueVis,
			//this.StabilizerTorqueVis
		];
		
		this.kiteMeshes = new THREE.Object3D();
		this.kiteMeshes.position.z -= this.height_of_c_g;
		for (let visiblePart of visibleKiteParts){
			this.kiteMeshes.add(visiblePart);
		}
		
		this.add(this.kiteMeshes);
		
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
			if(part.type == "FlatPlate")
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
		
		var cfk_mass = 0.250 * this.dimensions.x*this.dimensions.y*this.dimensions.z/0.012;
		var cfk_inertia = getInertia(cfk_mass, this.dimensions.x, this.dimensions.y, this.dimensions.z);
		
		var battery_mass = 0.180;
		var m_d_d_battery = battery_mass * 0.14*0.14; // mass * distance**2
		var battery_inertia = new THREE.Vector3();
		battery_inertia.set(0, m_d_d_battery, m_d_d_battery);
		
		var motors_mass = 2*0.100;
		var dist_motors = this.elevonData.distanceFromCenter;
		var motors_inertia = new THREE.Vector3();
		motors_inertia.set(motors_mass*dist_motors*dist_motors, motors_mass*0.14*0.14, motors_mass*(dist_motors*dist_motors + 0.14*0.14));
		
		var servo_mass = 2*0.020;
		var servo_x = 0.15;
		var servo_y = this.elevonData.distanceFromCenter;
		var servo_inertia = new THREE.Vector3();
		servo_inertia.set(servo_mass*servo_y*servo_y, servo_mass*servo_x*servo_x, servo_mass*(servo_y*servo_y + servo_x*servo_x));
		
		this.mass = spar_mass + xps_mass + cfk_mass + battery_mass + motors_mass + servo_mass;
		
		this.angular_inertia = spar_inertia.add(xps_inertia.add(cfk_inertia.add(battery_inertia.add(motors_inertia.add(servo_inertia)))));
	}
	
	setReflex(reflex_angle_in_radians){
		this.leftReflex.rotation.y = reflex_angle_in_radians;
		this.rightReflex.rotation.y = reflex_angle_in_radians;
	}
	
	setTannenbaumLengthAndBridleLength(tannenbaum_length, bridle_length){
		
		this.tannenbaum_length = tannenbaum_length;		//z-axis goes other direction than tannenbaum
		this.bridle_length = bridle_length;		//z-axis goes other direction than tannenbaum
		
		this.tannenbaumLeft.position.z = - this.tannenbaum_length*0.5;
		this.tannenbaumLeft.position.y = 0.25*this.dimensions.y;
		this.tannenbaumLeft.scale.z = tannenbaum_length;
		
		this.tannenbaumRight.position.z = - this.tannenbaum_length*0.5;
		this.tannenbaumRight.position.y = -0.25*this.dimensions.y;
		this.tannenbaumRight.scale.z = tannenbaum_length;
		
	}
	
	setPropellerDistance(prop_dist){
		this.elevonData.distanceFromCenter = prop_dist;
		
		this.elevonMeshLeft.position.y = this.elevonData.distanceFromCenter;
		this.elevonMeshRight.position.y = - this.elevonData.distanceFromCenter;
		
		this.leftPropeller.position.y = this.dimensions.y*0.5 * this.elevonData.distanceFromCenter;
		this.rightPropeller.position.y = -this.dimensions.y*0.5 * this.elevonData.distanceFromCenter;
		
		this.calculateAndSetAngularInertiaAndMass();
	}
	
	getRelativePositionOfLineKnotInKiteCoordinates(){
		
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
		var y_axis = new THREE.Vector3(this.matrix.elements[4], this.matrix.elements[5], this.matrix.elements[6]); // it's norm = 1
		var pos_of_kite = new THREE.Vector3(this.kiteMeshes.matrixWorld.elements[12], this.kiteMeshes.matrixWorld.elements[13], this.kiteMeshes.matrixWorld.elements[14]);
		var z_axis = new THREE.Vector3(this.matrix.elements[8], this.matrix.elements[9], this.matrix.elements[10]);
		var lineKnotDir = projectToComplementOfNormedVector(pos_of_kite.clone().add(z_axis.clone().multiplyScalar(-this.tannenbaum_length)), y_axis);
		lineKnotDir.normalize().multiplyScalar(this.bridle_length);
		
		var lineKnot = pos_of_kite.add(z_axis.clone().multiplyScalar(-this.tannenbaum_length)).sub(lineKnotDir);
		
		return lineKnot;
	}
	
	update(wind, line_tension, timestep_in_s){
		
		this.elevonMeshLeft.setAdditionalWind(this.leftPropeller.getForceAndCentreOfPressureInKiteCoords().force.multiplyScalar(-1));
		this.elevonMeshRight.setAdditionalWind(this.rightPropeller.getForceAndCentreOfPressureInKiteCoords().force.multiplyScalar(-1));
		
		let wind_vector = wind.getWindVector();
		let force = new THREE.Vector3();
		let torque = new THREE.Vector3(0, 0, 0);
		
		// adding all forces and torques
		for (let kitePart of this.aerodynamicKiteParts){
			let forceData = kitePart.getForceAndCentreOfPressureInKiteCoords(wind_vector);
			force.add(forceData.force);
			torque.add(forceData.pressure_centre.clone().cross(this.world2kite(forceData.force.clone())));
			if(kitePart.type == "FlatPlate") kitePart.torqueVis.set(forceData.pressure_centre, this.world2kite(forceData.force));
			
		}
		
		/*
		let wing1ForceData = this.simpleWing1.getForceAndCentreOfPressureInKiteCoords(wind_vector);
		this.Wing1TorqueVis.set(wing1ForceData.pressure_centre, this.world2kite(wing1ForceData.force));
		let wing2ForceData = this.simpleWing2.getForceAndCentreOfPressureInKiteCoords(wind_vector);
		this.Wing2TorqueVis.set(wing2ForceData.pressure_centre, this.world2kite(wing2ForceData.force));
		
		let stabForceData = this.stabilizerLeft.getForceAndCentreOfPressureInKiteCoords(wind_vector);
		this.StabilizerTorqueVis.set(stabForceData.pressure_centre, this.world2kite(stabForceData.force));
		*/
		
		
		//console.log("---");
		//console.log(wingForceData.force);
		// add line tension to force and torque
		let direction_of_line_tension_force = this.getPositionOfLineKnot().normalize();
		//console.log(direction_of_line_tension_force);
		let line_force_dir_in_kite_coords = this.world2kite(direction_of_line_tension_force.clone());
		//console.log(line_force_dir_in_kite_coords);
		force.add(direction_of_line_tension_force.multiplyScalar(-line_tension));
		torque.add(line_force_dir_in_kite_coords.clone().cross(this.getRelativePositionOfLineKnotInKiteCoordinates()).multiplyScalar(line_tension));
		this.lineTensionTorqueVis.set(this.getRelativePositionOfLineKnotInKiteCoordinates(), line_force_dir_in_kite_coords.multiplyScalar(-line_tension));
		
		// add gravity to force
		let gravity = new THREE.Vector3(-9.81, 0, 0);
		force.add(new THREE.Vector3(-10*this.mass, 0, 0));
		
		super.update(force, torque, timestep_in_s);
	}
}
