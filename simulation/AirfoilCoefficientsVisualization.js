"use strict";

class AirfoilCoefficientsVisualization{
	
	constructor(canvasName, wing1, wing2, diagramType){
		
		this.diagramType = diagramType;
		
		this.wing1 = wing1;
		this.wing2 = wing2;
		
		this.canvas = document.getElementById(canvasName);
		this.gl = this.canvas.getContext('experimental-webgl');
		
		var diagramLineCurrentIndex = 0;
		var max_line_segments = 20000;
		
		var lineWidth = 1;
		
		class DiagramLine{
			
			reset(minX, minY, maxX, maxY){
				this.line_index = 0;
				
				this.lastPoint = [1000000.0, 1000000.0];
				
				this.maxX = maxX;
				this.minX = minX;
				this.maxY = maxY;
				this.minY = minY;
				
				this.linearTrafo = [1.0, 0.0, 1.0, 0.0];
			}
			
			constructor(gl){
				this.gl = gl;
				this.index = diagramLineCurrentIndex;
				this.vertices = [];
				//500 lines, 1000 vertices, 2000 coordinates
				for (let i = 0; i < 4*max_line_segments; i++){
					this.vertices[i] = 0.0;
				}
				this.vertex_buffer = this.gl.createBuffer();
				this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertex_buffer);
				this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(this.vertices), this.gl.DYNAMIC_DRAW);
				
				this.color = [1.0, 1.0, 1.0];
				
				this.reset(2, 2);
				
				diagramLineCurrentIndex++;
			}
			
			addPoint(x, y){
				if( this.lastPoint[0] == 1000000.0){
					this.lastPoint = [x, y];
					return;
				}
				//if(x > this.maxX) this.maxX = x;
				//if(x < this.minX) this.minX = x;
				//if(y < this.minY) this.minY = y;
				//if(y > this.maxY) this.maxY = y;
				
				let maxY = Math.max(Math.abs(this.maxY), Math.abs(this.minY));
				let a = 1/(2*maxY);//1/(this.maxY-this.minY);
				let b = 0.5;//-this.minY/(this.maxY-this.minY);
				let m = 1/(this.maxX-this.minX);
				let n = -this.minX/(this.maxX-this.minX);
				this.transform(a, b, m, n);
				
				this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vertex_buffer);
				this.gl.bufferSubData(this.gl.ARRAY_BUFFER, 4*4*this.line_index, new Float32Array([this.lastPoint[0], this.lastPoint[1], x, y]));
				this.vertices[4*this.line_index] = this.lastPoint[0];
				this.vertices[4*this.line_index + 1] = this.lastPoint[1]
				this.vertices[4*this.line_index + 2] = x;
				this.vertices[4*this.line_index + 3] = y;
				this.lastPoint = [x, y];
				if(this.line_index < max_line_segments-1) this.line_index++;
			}
			
			transform(a, b, m, n){
				this.linearTrafo[0] = a;
				this.linearTrafo[1] = b;
				this.linearTrafo[2] = m;
				this.linearTrafo[3] = n;
			}
		}
		
		class Diagram{
			constructor(numLines, gl){
				this.gl = gl;
				this.diagramLines = [];
				for(let i = 0; i < numLines; i++){
					this.diagramLines[i] = new DiagramLine(this.gl);
				}
				
				let vertCode_lines =
				'attribute vec2 coordinates;' +
				'uniform vec3 color;'+
				'uniform vec4 linearTrafo;'+
				'varying vec3 vColor;'+
				'void main(void) {' +
				' gl_Position = vec4((coordinates.x*linearTrafo.z + linearTrafo.w)*2.0-1.0, (coordinates.y*linearTrafo.x + linearTrafo.y)*2.0-1.0, 0.0, 1.0);' +
			   	' vColor = color;'+
				'}';
				let fragCode_lines =
				'precision highp float;'+
				'varying vec3 vColor;'+
				'void main(void) {' +
				' gl_FragColor = vec4(vColor, 1.0);' +
				'}';
				
				let vertShader_lines = this.gl.createShader(this.gl.VERTEX_SHADER);
				this.gl.shaderSource(vertShader_lines, vertCode_lines);
				this.gl.compileShader(vertShader_lines);
				
				let fragShader_lines = this.gl.createShader(this.gl.FRAGMENT_SHADER);
				this.gl.shaderSource(fragShader_lines, fragCode_lines);
				this.gl.compileShader(fragShader_lines);
				
				this.shaderProgram_lines = this.gl.createProgram();
				this.gl.attachShader(this.shaderProgram_lines, vertShader_lines);
				this.gl.attachShader(this.shaderProgram_lines, fragShader_lines);
				this.gl.linkProgram(this.shaderProgram_lines);
				
				this.coord_lines = this.gl.getAttribLocation(this.shaderProgram_lines, "coordinates");
				this.color = this.gl.getUniformLocation(this.shaderProgram_lines, "color");
				this.linearTrafo = this.gl.getUniformLocation(this.shaderProgram_lines, "linearTrafo");
			}
			
			render(){
				this.gl.lineWidth(lineWidth);
				this.gl.enableVertexAttribArray(this.coord_lines);
				this.gl.useProgram(this.shaderProgram_lines);
				
				for(let i = 0; i < this.diagramLines.length; i++){
					this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.diagramLines[i].vertex_buffer);
					this.gl.vertexAttribPointer(this.coord_lines, 2, this.gl.FLOAT, false, 0, 0);
					this.gl.uniform3f(this.color, this.diagramLines[i].color[0], this.diagramLines[i].color[1], this.diagramLines[i].color[2]);
					this.gl.uniform4f(this.linearTrafo, this.diagramLines[i].linearTrafo[0], this.diagramLines[i].linearTrafo[1], this.diagramLines[i].linearTrafo[2], this.diagramLines[i].linearTrafo[3]);
					this.gl.drawArrays(this.gl.LINES, 0, 2*max_line_segments);
				}
				
				this.gl.disableVertexAttribArray(this.coord_lines);
			}
		}
		
		
		//DIAGRAM
		
		this.diagram = new Diagram(6, this.gl);
		this.diagram.diagramLines[0].color[0] = 0.0;
		this.diagram.diagramLines[0].color[1] = 0.0;
		this.diagram.diagramLines[0].color[2] = 0.0;
		
		this.diagram.diagramLines[1].color[0] = 0.0;
		this.diagram.diagramLines[1].color[1] = 0.0;
		
		this.diagram.diagramLines[2].color[1] = 0.0;
		this.diagram.diagramLines[2].color[2] = 0.0;
		
		this.diagram.diagramLines[3].color[0] = 0.0;
		this.diagram.diagramLines[3].color[2] = 0.0;
		
		this.diagram.diagramLines[4].color[1] = 0.0;
		
		this.diagram.diagramLines[5].color[0] = 0.5;
		this.diagram.diagramLines[5].color[1] = 0.5;
		this.diagram.diagramLines[5].color[2] = 0.5;
		
		
		
		
		
		if(this.diagramType == "L,D"){
			this.diagram.diagramLines[3].reset(0, 0, Math.PI, 2); // graph Lift
			this.diagram.diagramLines[1].reset(0, 0, Math.PI, 2); // graph Drag
			this.diagram.diagramLines[0].reset(0, 0, Math.PI, 2); // coordinate system
			
			this.diagram.diagramLines[0].addPoint(0, 0);
			this.diagram.diagramLines[0].addPoint(Math.PI, 0);
			this.diagram.diagramLines[0].addPoint(Math.PI, 0.1);
			this.diagram.diagramLines[0].addPoint(Math.PI, 0);
			this.diagram.diagramLines[0].addPoint(0, 0);
			this.diagram.diagramLines[0].addPoint(0, 2);
			this.diagram.diagramLines[0].addPoint(0.06, 2);
			this.diagram.diagramLines[0].addPoint(0, 2);
			this.diagram.diagramLines[0].addPoint(0, -2);
			this.diagram.diagramLines[0].addPoint(0.06, -2);
			this.diagram.diagramLines[0].addPoint(0, -2);
			
			
			
			let resolution = 400;
			for(let i = 0; i < resolution; i++){
				let AoA = i * Math.PI / resolution;
				let C_L = lift_coefficient(AoA, this.wing1);
				let C_D = drag_coefficient(AoA, 2*this.wing1.dimensions.y/this.wing1.dimensions.x, this.wing1);
				this.diagram.diagramLines[3].addPoint(AoA, C_L);
				this.diagram.diagramLines[1].addPoint(AoA, C_D);
			}
		}else if(this.diagramType = "L/D"){
			this.diagram.diagramLines[0].reset(0, -2, 0.5, 2); // coordinate system
			this.diagram.diagramLines[1].reset(0, -2, 0.5, 2); // graph
			//this.diagram.diagramLines[5].reset(0, -2, 0.5, 2); // cross hair
			
			this.diagram.diagramLines[0].addPoint(0, 0);
			this.diagram.diagramLines[0].addPoint(0.5, 0);
			this.diagram.diagramLines[0].addPoint(0.5, 0.1);
			this.diagram.diagramLines[0].addPoint(0.5, 0);
			this.diagram.diagramLines[0].addPoint(0, 0);
			this.diagram.diagramLines[0].addPoint(0, 2);
			this.diagram.diagramLines[0].addPoint(0.01, 2);
			this.diagram.diagramLines[0].addPoint(0, 2);
			this.diagram.diagramLines[0].addPoint(0, -2);
			this.diagram.diagramLines[0].addPoint(0.01, -2);
			this.diagram.diagramLines[0].addPoint(0, -2);
			for(let i = -500; i < 1000; i++){
				let AoA = i * 0.5 * Math.PI / 1000;
				let C_L = lift_coefficient(AoA, this.wing1);
				let C_D = drag_coefficient(AoA, 2*this.wing1.dimensions.y/this.wing1.dimensions.x, this.wing1);
				this.diagram.diagramLines[1].addPoint(C_D, C_L);
			}
		}
		
		var vis = this;
		
		var animate = function(time) {
			
			vis.gl.disable(vis.gl.DEPTH_TEST);
			vis.gl.depthFunc(vis.gl.LEQUAL);
			vis.gl.clearColor(1.0, 1.0, 1.0, 1.0);
			vis.gl.clearDepth(1.0);
			
			vis.gl.viewport(0.0, 0.0, 400, 300);
			vis.gl.clear(vis.gl.COLOR_BUFFER_BIT | vis.gl.DEPTH_BUFFER_BIT);
			
			
			if(vis.diagramType == "L,D"){
				vis.diagram.diagramLines[2].reset(0, 0, Math.PI, 2);
				
				vis.diagram.diagramLines[2].addPoint(vis.wing1.AoA_for_vis, -2);
				vis.diagram.diagramLines[2].addPoint(vis.wing1.AoA_for_vis, 2);
			}else if(vis.diagramType = "L/D"){
				vis.diagram.diagramLines[2].reset(0, -2, 0.5, 2);
				
				let AoA = vis.wing1.AoA_for_vis;
				let C_L = lift_coefficient(AoA, vis.wing1);
				let C_D = drag_coefficient(AoA, 2*vis.wing1.dimensions.y/vis.wing1.dimensions.x, vis.wing1);
				
				vis.diagram.diagramLines[2].addPoint(0, C_L);
				vis.diagram.diagramLines[2].addPoint(0.51, C_L);
				
				vis.diagram.diagramLines[2].addPoint(0.51, -21);
				
				vis.diagram.diagramLines[2].addPoint(C_D, -21);
				vis.diagram.diagramLines[2].addPoint(C_D, 2);
			}
			
			
			vis.diagram.render();
			
			setTimeout(() => {  window.requestAnimationFrame(animate); }, 100);
		 }
		 animate(0);
		
	}
	
}
