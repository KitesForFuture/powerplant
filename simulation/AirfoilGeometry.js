"use strict";

class AirfoilGeometry extends THREE.BufferGeometry{
	
	constructor(qr){
	
		super();
		
		let verts2d = [0.750,0.022871,0.727656,0.027265,0.702765,0.031862,0.677821,0.036588,0.652823,0.041363,0.627833,0.046105,0.602906,0.050751,0.578063,0.055247,0.553314,0.059556,0.528651,0.063647,0.504066,0.067492,0.479559,0.071064,0.455136,0.074336,0.430800,0.077284,0.406551,0.079883,0.382390,0.082104,0.358334,0.083922,0.334384,0.085306,0.310558,0.086230,0.286872,0.086662,0.263355,0.086565,0.240034,0.085905,0.216961,0.084647,0.194188,0.082749,0.171770,0.080167,0.149782,0.076847,0.128343,0.072736,0.107430,0.067737,0.087558,0.061898,0.069038,0.055276,0.052371,0.048071,0.038174,0.040671,0.026880,0.033580,0.018562,0.027320,0.012348,0.021730,0.007422,0.016346,0.004009,0.011658,0.001790,0.007588,0.000377,0.003407,0.000003,0.000294,0.000233,-0.002388,0.001626,-0.005884,0.002928,-0.007773,0.004907,-0.009858,0.007893,-0.012055,0.011686,-0.014137,0.016767,-0.016273,0.023630,-0.018462,0.033232,-0.020806,0.046725,-0.023292,0.064545,-0.025707,0.086065,-0.027949,0.109654,-0.029917,0.134140,-0.031597,0.159022,-0.033035,0.184086,-0.034277,0.209229,-0.035350,0.234402,-0.036277,0.259592,-0.037075,0.284767,-0.037755,0.309924,-0.038322,0.335073,-0.038781,0.360220,-0.039137,0.385377,-0.039386,0.410548,-0.039532,0.435718,-0.039573,0.460901,-0.039509,0.486103,-0.039336,0.511308,-0.039053,0.536494,-0.038658,0.561667,-0.038147,0.586817,-0.037518,0.611956,-0.036765,0.637083,-0.035888,0.662181,-0.034880,0.687248,-0.033735,0.712277,-0.032448,0.737248,-0.031009,0.75,-0.029402];
		let offset = 0.5*0.75;
		let scale = 1.3333;
		
		let maxY = verts2d[1];
		let minY = verts2d[1];
		for(let i = 0; i < verts2d.length/2; i++){
			if(verts2d[2*i+1] > maxY) maxY = verts2d[2*i+1];
			if(verts2d[2*i+1] < minY) minY = verts2d[2*i+1];
		}
		let yScaleFactor = 1/(maxY-minY);
		
		if(qr){
			verts2d = [1.000000,0.000000,0.989670,-0.000404,0.970166,-0.000609,0.948167,-0.000118,0.924260,0.001066,0.899973,0.002842,0.875609,0.005144,0.851029,0.007949,0.826402,0.011190,0.801795,0.014796,0.777151,0.018705,0.75,0.022871,0.75,-0.029402,0.787042,-0.027617,0.811828,-0.025639,0.836488,-0.023455,0.861030,-0.021048,0.885354,-0.018398,0.909527,-0.015452,0.933057,-0.012226,0.954383,-0.008909,0.973235,-0.005517,0.990287,-0.001989,1.000000,0.000000];
			offset = 0.875;
			scale = 4;
		}
		for(let i = 0; i < verts2d.length/2; i++){
			verts2d[2*i+1] *= yScaleFactor;
		}
		
		let verts3d = [];
		
		for(let i = 0; i < verts2d.length/2; i++){
			
			verts3d[18*i+0] = scale*(-verts2d[2*i+0]+offset);
			verts3d[18*i+2] = verts2d[2*i+1];
			verts3d[18*i+1] = 0.5;
			
			verts3d[18*i+3] = scale*(-verts2d[2*i+0]+offset);
			verts3d[18*i+5] = verts2d[2*i+1];
			verts3d[18*i+4] = -0.5;
			
			verts3d[18*i+6] = scale*(-verts2d[(2*i+2+0)%verts2d.length]+offset);
			verts3d[18*i+8] = verts2d[(2*i+2+1)%verts2d.length];
			verts3d[18*i+7] = -0.5;
			
			verts3d[18*i+9] = scale*(-verts2d[2*i+0]+offset);
			verts3d[18*i+11] = verts2d[2*i+1];
			verts3d[18*i+10] = 0.5;
			
			verts3d[18*i+12] = scale*(-verts2d[(2*i+2+0)%verts2d.length]+offset);
			verts3d[18*i+14] = verts2d[(2*i+2+1)%verts2d.length];
			verts3d[18*i+13] = -0.5;
			
			verts3d[18*i+15] = scale*(-verts2d[(2*i+2+0)%verts2d.length]+offset);
			verts3d[18*i+17] = verts2d[(2*i+2+1)%verts2d.length];
			verts3d[18*i+16] = 0.5;
			
		}
		for(let i = 0; i < verts2d.length/2; i++){
			verts3d[9*verts2d.length + 18*i + 0] = 0;
			verts3d[9*verts2d.length + 18*i + 1] = -0.5;
			verts3d[9*verts2d.length + 18*i + 2] = 0;
			
			verts3d[9*verts2d.length + 18*i + 3] = scale*(-verts2d[(2*i+2+0)%verts2d.length]+offset);
			verts3d[9*verts2d.length + 18*i + 4] = -0.5;
			verts3d[9*verts2d.length + 18*i + 5] = verts2d[(2*i+2+1)%verts2d.length];
			
			verts3d[9*verts2d.length + 18*i + 6] = scale*(-verts2d[2*i+0]+offset);
			verts3d[9*verts2d.length + 18*i + 7] = -0.5;
			verts3d[9*verts2d.length + 18*i + 8] = verts2d[2*i+1];
			
			verts3d[9*verts2d.length + 18*i + 9] = 0;
			verts3d[9*verts2d.length + 18*i + 10] = 0.5;
			verts3d[9*verts2d.length + 18*i + 11] = 0;
			
			verts3d[9*verts2d.length + 18*i + 12] = scale*(-verts2d[2*i+0]+offset);
			verts3d[9*verts2d.length + 18*i + 13] = 0.5;
			verts3d[9*verts2d.length + 18*i + 14] = verts2d[2*i+1];
			
			verts3d[9*verts2d.length + 18*i + 15] = scale*(-verts2d[(2*i+2+0)%verts2d.length]+offset);
			verts3d[9*verts2d.length + 18*i + 16] = 0.5;
			verts3d[9*verts2d.length + 18*i + 17] = verts2d[(2*i+2+1)%verts2d.length];
		}

		// create a simple square shape. We duplicate the top left and bottom right
		// vertices because each vertex needs to appear once per triangle.
		const vertices = new Float32Array( verts3d );

		// itemSize = 3 because there are 3 values (components) per vertex
		this.setAttribute( 'position', new THREE.BufferAttribute( vertices, 3 ) );
		this.computeVertexNormals ();
	}
}
