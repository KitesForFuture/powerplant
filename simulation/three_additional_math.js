
function vec2Str(vector){
	return "vec = (" + vector.x + ", " + vector.y + ", " + vector.z + ")";
}

function mat2Str(n){
	let m = n.elements;
	return "vec = \n(" + m[0] + ", " + m[1] + ", " + m[2] + "\n"
	+ m[3] + ", " + m[4] + ", " + m[5] + "\n"
	+ m[6] + ", " + m[7] + ", " + m[8] + ")";
}

function projectToNormedVector(a, n){
	return n.clone().multiplyScalar(a.clone().dot(n));
}

function projectToComplementOfNormedVector(a, n){
	return a.clone().sub(projectToNormedVector(a,n));
}

function angle(vec1, vec2){
	var normed_vec1 = vec1.clone().normalize();
	var normed_vec2 = vec2.clone().normalize();
	return acos_clamp(normed_vec1.dot(normed_vec2));
}

function getRotationMatrixPart(matrix4){
	var m = matrix4.elements;
	var mat = new THREE.Matrix3();
	mat.set(m[0], m[4], m[8], m[1], m[5], m[9], m[2], m[6], m[10]);
	return mat;
}
/*
function setFromRotationMatrix(object){
	
}
*/
function lift(c_l, v, A){
	return c_l * 0.5 * 1.2 * v * v * A;
}

function drag(c_d, v, A){
	return c_d * 0.5 * 1.2 * v * v * A;
}

function angle_of_attack(normal, unit_wind_direction){
	return asin_clamp(normal.dot(unit_wind_direction));
}
// in world coordinates
// can be 0
function lift_direction(normal, unit_wind_direction){
	return (unit_wind_direction.clone().cross(normal)).cross(unit_wind_direction);
}
//in world coordinates
function drag_direction(unit_wind_direction){
	return unit_wind_direction;
}

function lift_coefficient(angle_of_attack, obj){
	//if(obj.material.color.r == 1.0 && obj.material.color.g == 1.0 && obj.material.color.b == 1.0)
		//console.log("lift = " + (Math.sin(2 * angle_of_attack) + Math.exp(-49*(angle_of_attack-0.25)*(angle_of_attack-0.25))));
	return Math.sin(2 * angle_of_attack) + Math.exp(-49*(angle_of_attack-0.25)*(angle_of_attack-0.25));
}

function drag_coefficient(angle_of_attack, aspect_ratio, obj){
	//return 1 - Math.cos(2 * angle_of_attack);
	//console.log(obj);
	//if(obj.material.color.r == 1.0 && obj.material.color.g == 1.0 && obj.material.color.b == 1.0)
		//console.log("ar = " + aspect_ratio + ", zero_lift Drag = 0.01, AoA-Drag = " + (1 - Math.cos(2 * angle_of_attack)) + ", AR-Drag = " + lift_coefficient(angle_of_attack, obj)*lift_coefficient(angle_of_attack, obj)/(2.5 /* pi*oswald_efficiency_number */ *aspect_ratio));
	
	return 0.01 /*drag at zero lift*/ + 1 - Math.cos(2 * angle_of_attack) /*drag depending on angle of attack*/ + lift_coefficient(angle_of_attack, obj)*lift_coefficient(angle_of_attack, obj)/(2.5 /* pi*oswald_efficiency_number */ *aspect_ratio);
}



function normalize_matrix(b){
	a = b.elements;
	// Gram-Schmidt orthogonalization
	// (direction of first column stays constant and always only the latter two are rotated)
	
	var norm = Math.sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
	for(var i = 0; i < 3; i++){
		a[i] /= norm;
	}
	
	var scalarProd = a[0]*a[3]+a[1]*a[4]+a[2]*a[5];
	for(var i = 0; i < 3; i++){
		a[3+i] -= scalarProd*a[i];
	}
	
	norm = Math.sqrt(a[3]*a[3] + a[4]*a[4] + a[5]*a[5]);
	for(var i = 0; i < 3; i++){
		a[3+i] /= norm;
	}
	
	scalarProd = a[0]*a[6]+a[1]*a[7]+a[2]*a[8];
	for(var i = 0; i < 3; i++){
		a[6+i] -= scalarProd*a[i];
	}
	
	scalarProd = a[3]*a[6]+a[4]*a[7]+a[5]*a[8];
	for(var i = 0; i < 3; i++){
		a[6+i] -= scalarProd*a[3+i];
	}
	
	norm = Math.sqrt(a[6]*a[6] + a[7]*a[7] + a[8]*a[8]);
	for(var i = 0; i < 3; i++){
		a[6+i] /= norm;
	}
}
