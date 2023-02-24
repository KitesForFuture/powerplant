#include "../../common/num_config_vars.h"

#define EXAMPLE_ESP_WIFI_SSID      "Kite-Config"
#define EXAMPLE_ESP_WIFI_PASS      "password"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       1


static const char *TAG = "wifi softAP";
static void (*write_callback)(float*);
static void (*init_callback)(float*);
static void (*debugging_data_callback)(float*);

// ****** GET WEBSITE ******

static esp_err_t kite_config_html_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Getting html config page");
	
	esp_err_t error;
    const char* response = (const char*) req->user_ctx;
    error = httpd_resp_send(req, response, strlen(response));
    return error;
}

static const httpd_uri_t kite_config_get_html = {
    .uri       = "/config",
    .method    = HTTP_GET,
    .handler   = kite_config_html_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "	<!DOCTYPE html>\
<html>\
<head>\
					<style>\
					.button {\
					  border: none;\
					  color: white;\
					  padding: 15px 32px;\
					  text-align: center;\
					  text-decoration: none;\
					  display: inline-block;\
					  font-size: 16px;\
					  margin: 4px 2px;\
					  cursor: pointer;\
					}\
					\
					.button1 {background-color: #4CAF50;} /* Green */\
					\
					.slidecontainer {\
					  width: 100%;\
					}\
\
					.slider {\
					  -webkit-appearance: none;\
					  width: 100%;\
					  height: 25px;\
					  background: #d3d3d3;\
					  outline: none;\
					  opacity: 0.7;\
					  -webkit-transition: .2s;\
					  transition: opacity .2s;\
					}\
\
					.slider:hover {\
					  opacity: 1;\
					}\
\
					.slider::-webkit-slider-thumb {\
					  -webkit-appearance: none;\
					  appearance: none;\
					  width: 25px;\
					  height: 25px;\
					  background: #04AA6D;\
					  cursor: pointer;\
					}\
\
					.slider::-moz-range-thumb {\
					  width: 25px;\
					  height: 25px;\
					  background: #04AA6D;\
					  cursor: pointer;\
					}\
\
					tr td:last-child {\
						width: 1%;\
						white-space: nowrap;\
					}\
					tr td:first-child {\
						width: 1%;\
						white-space: nowrap;\
					}\
					</style>\
</head>\
<body>\
					\n\
					<canvas width = \"1800\" height = \"700\" id = \"my_Canvas\"></canvas>\n\
					<label for=\"fnameDiagram\">Save Diagram As: </label>\n\
					<input type=\"text\" id=\"filenameDiagram\" name=\"filename\">\n\
					<button type=\"button\" id=\"saveButtonDiagram\">Save Diagram</button><br><br>\n\
					<span id=\"autogeneratedLineExtrema\"></span>\n\
					<button id=\"pause_button\" class=\"button button1\">Pause</button><br>\n\
					<input type=\"range\" min=\"1\" max=\"5\" value=\"1\" class=\"slider\" id=\"lineWidth_slider\"><br>\n\
					black, blue, red, green, pink, <br>\n\
					<span id=\"autogeneratedHTML\"></span>\n\
					<label for=\"fname\">Save Configuration As: </label>\n\
					<input type=\"text\" id=\"filename\" name=\"filename\">\n\
					<button type=\"button\" id=\"saveButton\">Save</button><br><br>\n\
					Load configuration: <input type=\"file\" id=\"myfile\" name=\"myfile\"><br><br>\n\
					\n\
					\n\
					<script>\n\
						\n\
						/*============= Creating a canvas =================*/\n\
						var canvas = document.getElementById('my_Canvas');\n\
						gl = canvas.getContext('experimental-webgl');\n\
						\n\
						var UIstringDiagram = \"\";\n\
						 /*============ Defining and storing the geometry =========*/\n\
						\n\
						var diagramLineCurrentIndex = 0;\n\
						var max_line_segments = 20000;\n\
						\n\
						var receiving_data_points_paused = false;\n\
						\n\
						var lineWidth = 1;\n\
						\n\
						window[\"lineWidth_slider\"].oninput = function() {\n\
							lineWidth = this.value;\n\
						}\n\
						window[\"pause_button\"].onclick = function(){\n\
							receiving_data_points_paused = !receiving_data_points_paused;\n\
							if(receiving_data_points_paused) this.value = \"paused\"; else this.value = \"receiving\";\n\
						}\n\
						\n\
						class DiagramLine{\n\
							constructor(){\n\
								this.index = diagramLineCurrentIndex;\n\
								this.vertices = [];\n\
								//500 lines, 1000 vertices, 2000 coordinates\n\
								for (let i = 0; i < 4*max_line_segments; i++){\n\
									this.vertices[i] = 0.0;\n\
								}\n\
								this.vertex_buffer = gl.createBuffer();\n\
								gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);\n\
								gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(this.vertices), gl.DYNAMIC_DRAW);\n\
								\n\
								this.color = [1.0, 1.0, 1.0];\n\
								this.line_index = 0;\n\
								\n\
								this.lastPoint = [1000000.0, 1000000.0];\n\
								this.maxX = 0.00000001;\n\
								this.minX = 0;\n\
								this.maxY = 0.00000001;\n\
								this.minY = 0;\n\
								this.linearTrafo = [1.0, 0.0, 1.0, 0.0];\n\
								\n\
								UIstringDiagram += \"<div>Line \" + this.index + \": [<span id=\\\"line\" + this.index + \"min\\\"></span>, <span id=\\\"line\" + this.index + \"max\\\"></span>]</div>\";\n\
								\n\
								document.getElementById(\"autogeneratedLineExtrema\").innerHTML = UIstringDiagram;\n\
								diagramLineCurrentIndex++;\n\
							}\n\
							\n\
							addPoint(x, y){\n\
								if(receiving_data_points_paused) return;\n\
								if(this.lastPoint[0] == 1000000.0){\n\
									this.lastPoint = [x, y];\n\
									this.maxX = x+0.00000000001;\n\
									this.minX = x;\n\
									this.maxY = y+0.00000000001;\n\
									this.minY = y;\n\
									return;\n\
								}\n\
								if(x > this.maxX) this.maxX = x;\n\
								if(x < this.minX) this.minX = x;\n\
								if(y < this.minY) this.minY = y;\n\
								if(y > this.maxY) this.maxY = y;\n\
								window[\"line\" + this.index + \"min\"].innerHTML = this.minY.toFixed(3);\n\
								window[\"line\" + this.index + \"max\"].innerHTML = this.maxY.toFixed(3);\n\
								\n\
								let maxY = Math.max(Math.abs(this.maxY), Math.abs(this.minY));\n\
								let a = 1/(2*maxY);//1/(this.maxY-this.minY);\n\
								let b = 0.5;//-this.minY/(this.maxY-this.minY);\n\
								let m = 1/(this.maxX-this.minX);\n\
								let n = -this.minX/(this.maxX-this.minX);\n\
								this.transform(a, b, m, n);\n\
								\n\
								gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);\n\
								gl.bufferSubData(gl.ARRAY_BUFFER, 4*4*this.line_index, new Float32Array([this.lastPoint[0], this.lastPoint[1], x, y]));\n\
								this.vertices[4*this.line_index] = this.lastPoint[0];\n\
								this.vertices[4*this.line_index + 1] = this.lastPoint[1]\n\
								this.vertices[4*this.line_index + 2] = x;\n\
								this.vertices[4*this.line_index + 3] = y;\n\
								this.lastPoint = [x, y];\n\
								if(this.line_index < max_line_segments-1) this.line_index++;\n\
							}\n\
							\n\
							transform(a, b, m, n){\n\
								this.linearTrafo[0] = a;\n\
								this.linearTrafo[1] = b;\n\
								this.linearTrafo[2] = m;\n\
								this.linearTrafo[3] = n;\n\
							}\n\
						}\n\
						\n\
						class Diagram{\n\
							constructor(numLines){\n\
								this.diagramLines = [];\n\
								for(let i = 0; i < numLines; i++){\n\
									this.diagramLines[i] = new DiagramLine();\n\
								}\n\
								\n\
								let vertCode_lines =\n\
								'attribute vec2 coordinates;' +\n\
								'uniform vec3 color;'+\n\
								'uniform vec4 linearTrafo;'+\n\
								'varying vec3 vColor;'+\n\
								'void main(void) {' +\n\
								' gl_Position = vec4((coordinates.x*linearTrafo.z + linearTrafo.w)*2.0-1.0, (coordinates.y*linearTrafo.x + linearTrafo.y)*2.0-1.0, 0.0, 1.0);' +\n\
							   	' vColor = color;'+\n\
								'}';\n\
								let fragCode_lines =\n\
								'precision highp float;'+\n\
								'varying vec3 vColor;'+\n\
								'void main(void) {' +\n\
								' gl_FragColor = vec4(vColor, 1.0);' +\n\
								'}';\n\
								\n\
								let vertShader_lines = gl.createShader(gl.VERTEX_SHADER);\n\
								gl.shaderSource(vertShader_lines, vertCode_lines);\n\
								gl.compileShader(vertShader_lines);\n\
								\n\
								let fragShader_lines = gl.createShader(gl.FRAGMENT_SHADER);\n\
								gl.shaderSource(fragShader_lines, fragCode_lines);\n\
								gl.compileShader(fragShader_lines);\n\
								\n\
								this.shaderProgram_lines = gl.createProgram();\n\
								gl.attachShader(this.shaderProgram_lines, vertShader_lines);\n\
								gl.attachShader(this.shaderProgram_lines, fragShader_lines);\n\
								gl.linkProgram(this.shaderProgram_lines);\n\
								\n\
								this.coord_lines = gl.getAttribLocation(this.shaderProgram_lines, \"coordinates\");\n\
								this.color = gl.getUniformLocation(this.shaderProgram_lines, \"color\");\n\
								this.linearTrafo = gl.getUniformLocation(this.shaderProgram_lines, \"linearTrafo\");\n\
							}\n\
							\n\
							render(){\n\
								gl.lineWidth(lineWidth);\n\
								gl.enableVertexAttribArray(this.coord_lines);\n\
								gl.useProgram(this.shaderProgram_lines);\n\
								\n\
								for(let i = 0; i < this.diagramLines.length; i++){\n\
									gl.bindBuffer(gl.ARRAY_BUFFER, this.diagramLines[i].vertex_buffer);\n\
									gl.vertexAttribPointer(this.coord_lines, 2, gl.FLOAT, false, 0, 0);\n\
									gl.uniform3f(this.color, this.diagramLines[i].color[0], this.diagramLines[i].color[1], this.diagramLines[i].color[2]);\n\
									gl.uniform4f(this.linearTrafo, this.diagramLines[i].linearTrafo[0], this.diagramLines[i].linearTrafo[1], this.diagramLines[i].linearTrafo[2], this.diagramLines[i].linearTrafo[3]);\n\
									gl.drawArrays(gl.LINES, 0, 2*max_line_segments);\n\
								}\n\
								\n\
								gl.disableVertexAttribArray(this.coord_lines);\n\
							}\n\
						}\n\
						\n\
						//DIAGRAM\n\
						\n\
						var diagram = new Diagram(6);\n\
						diagram.diagramLines[0].color[0] = 0.0;\n\
						diagram.diagramLines[0].color[1] = 0.0;\n\
						diagram.diagramLines[0].color[2] = 0.0;\n\
						\n\
						diagram.diagramLines[1].color[0] = 0.0;\n\
						diagram.diagramLines[1].color[1] = 0.0;\n\
						\n\
						diagram.diagramLines[2].color[1] = 0.0;\n\
						diagram.diagramLines[2].color[2] = 0.0;\n\
						\n\
						diagram.diagramLines[3].color[0] = 0.0;\n\
						diagram.diagramLines[3].color[2] = 0.0;\n\
						\n\
						diagram.diagramLines[4].color[1] = 0.0;\n\
						\n\
						diagram.diagramLines[5].color[0] = 0.5;\n\
						diagram.diagramLines[5].color[1] = 0.5;\n\
						diagram.diagramLines[5].color[2] = 0.5;\n\
						\n\
						\n\
						\n\
						\n\
						\n\
						\n\
						function saveFile(filename, text){\n\
							var pom = document.createElement('a');\n\
							pom.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));\n\
							pom.setAttribute('download', filename);\n\
							\n\
							if(document.createEvent){\n\
								var event = document.createEvent('MouseEvents');\n\
								event.initEvent('click', true, true);\n\
								pom.dispatchEvent(event);\n\
							}else{\n\
								pom.click();\n\
							}\n\
						}\n\
						document.getElementById('saveButtonDiagram').onclick = function(){\n\
							saveFile(\"\" + document.getElementById('filenameDiagram').value + \".kitediagram\", JSON.stringify(diagram));\n\
						};\n\
						\n\
						document.getElementById('saveButton').onclick = function(){\n\
							saveFile(\"\" + document.getElementById('filename').value + \".kiteconfig2\", JSON.stringify(configValues));\n\
						};\n\
						\n\
						document.getElementById('myfile').oninput = function(event){\n\
							let fileReader = new FileReader(); \n\
							fileReader.readAsText(document.getElementById('myfile').files[0]);\n\
							fileReader.onload = function() {\n\
								let result = JSON.parse(fileReader.result);\n\
								for (i = 0; i < Math.min(configValues.length, result.length); i++) {\n\
									configValues[i] = result[i];\n\
								}\n\
								updateConfigValuesInHTML();\n\
								uploadConfig();\n\
							}; \n\
							fileReader.onerror = function() {\n\
							  alert(fileReader.error);\n\
							};\n\
						};\n\
						\n\
						\n\
						function downloadConfig(){\n\
							var xhr = new XMLHttpRequest();\n\
							xhr.open('GET', 'get_config', true);\n\
							xhr.responseType = 'text';\n\
							xhr.onload = function(e){\n\
								const myArray = xhr.response.split(\",\");\n\
								for(let i = 0; i < numConfigValues; i++){\n\
									configValues[i] = parseFloat(myArray[i]);\n\
									if(i==6) configValues[i] *= 0.000001;//because (s)prinf only prints 5 digits after the comma\n\
								}\n\
								//console.log(configValues);\n\
								updateConfigValuesInHTML();\n\
							}\n\
							xhr.send();\n\
						}\n\
						\n\
						var debuggingData = [0, 0, 0, 0, 0, 0];\n\
						var debuggingDataInitialized = false;\n\
						ready_to_download_debugging_data = true;\n\
						function downloadDebuggingData(){\n\
							var xhr = new XMLHttpRequest();\n\
							xhr.open('GET', 'get_debugging_data', true);\n\
							xhr.responseType = 'text';\n\
							xhr.onload = function(e){\n\
								const myArray = xhr.response.split(\",\");\n\
								for(let i = 0; i < 6; i++){\n\
									debuggingData[i] = parseFloat(myArray[i]);\n\
									if(debuggingData[i] != 1000000) debuggingDataInitialized = true;\n\
								}\n\
								//console.log(\"TODO\");\n\
								ready_to_download_debugging_data = true;\n\
								if(debuggingDataInitialized) updateDebuggingDataGraph();\n\
							}\n\
							xhr.send();\n\
						}\n\
						\n\
						function uploadConfig(){\n\
							\n\
							let config_string = \"\";\n\
							for(let i = 0; i < numConfigValues-1; i++){\n\
								config_string += configValues[i] + \",\";\n\
							}\n\
							config_string += configValues[numConfigValues-1] + \",\";\n\
							sendData(config_string, 'uploadConfig',\n\
							() => {  console.log('succeeded sending config'); downloadConfig(); },\n\
							() => {  console.log('failed sending config'); downloadConfig(); }  );\n\
						}\n\
						\n\
						function sendData(data, filename, successCallback, failedCallback){\n\
							var xmlhttp;\n\
							xmlhttp = new XMLHttpRequest();\n\
							xmlhttp.onreadystatechange = function() {\n\
								if (this.readyState == 4 && this.status != 200) {\n\
									failedCallback();\n\
								}\n\
								if(this.readyState == 4 && this.status == 200){\n\
									successCallback();\n\
								}\n\
							};\n\
							xmlhttp.open(\"POST\",filename,true);\n\
							xmlhttp.send(\"\" + data);\n\
						}\n\
						\n\
						\n\
						var numConfigValues = " NUM_CONFIG_FLOAT_VARS_string " + " NUM_GS_CONFIG_FLOAT_VARS_string ";\n\
						var numActuatorsWithoutConfig = 2;\n\
						var configValues = new Array(numConfigValues).fill(0);\n\
						\n\
						var jsFunctions = new Array(numConfigValues + numActuatorsWithoutConfig);\n\
						\n\
						var UIstring = \"\";\n\
						\n\
						function addPIDConstant(name, index){\n\
							UIstring += \"\\\n\
								<tr>\\\n\
									<td><p>\" + name + \"=<br><span id=\\\"value\" + index + \"\\\"></span></p></td>\\\n\
									<td><button id=\\\"Down\" + index + \"Button\\\" class=\\\"button button1\\\">&#8681</button>\\\n\
									<button id=\\\"Up\" + index + \"Button\\\" class=\\\"button button1\\\">&#8679</button></td>\\\n\
									<td></td>\\\n\
								</tr>\\\n\
							\";\n\
							jsFunctions[index] = function(){\n\
								window[\"value\" + index] = document.getElementById(\"value\" + index);\n\
								//configValues[index] = 1;\n\
								window[\"value\" + index].innerHTML = configValues[index];\n\
								\n\
								document.getElementById(\"Up\" + index + \"Button\").onclick = function() {\n\
									configValues[index] *= 1.1;\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(2);\n\
									uploadConfig();\n\
								}\n\
								document.getElementById(\"Down\" + index + \"Button\").onclick = function() {\n\
									configValues[index] /= 1.1;\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(2);\n\
									uploadConfig();\n\
								}\n\
							};\n\
						}\n\
						\n\
						function addDegrees(name, index){\n\
							UIstring += \"\\\n\
								<tr>\\\n\
									<td><p>\" + name + \"=<br><span id=\\\"value\" + index + \"\\\"></span></p></td>\\\n\
									<td><button id=\\\"Down\" + index + \"Button\\\" class=\\\"button button1\\\">&#8681</button>\\\n\
									<button id=\\\"Up\" + index + \"Button\\\" class=\\\"button button1\\\">&#8679</button></td>\\\n\
									<td></td>\\\n\
								</tr>\\\n\
							\";\n\
							jsFunctions[index] = function(){\n\
								window[\"value\" + index] = document.getElementById(\"value\" + index);\n\
								//configValues[index] = 45;\n\
								window[\"value\" + index].innerHTML = configValues[index];\n\
								\n\
								document.getElementById(\"Up\" + index + \"Button\").onclick = function() {\n\
									configValues[index] += 1;\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(0);\n\
									uploadConfig();\n\
								}\n\
								document.getElementById(\"Down\" + index + \"Button\").onclick = function() {\n\
									configValues[index] -= 1;\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(0);\n\
									uploadConfig();\n\
								}\n\
							};\n\
						}\n\
						\n\
						function addHeight(name, index, lowerBound){\n\
							UIstring += \"\\\n\
								<tr>\\\n\
									<td><p>\" + name + \"=<br><span id=\\\"value\" + index + \"\\\"></span></p></td>\\\n\
									<td><button id=\\\"Down\" + index + \"Button\\\" class=\\\"button button1\\\">&#8681</button>\\\n\
									<button id=\\\"Up\" + index + \"Button\\\" class=\\\"button button1\\\">&#8679</button></td>\\\n\
									<td></td>\\\n\
								</tr>\\\n\
							\";\n\
							jsFunctions[index] = function(){\n\
								window[\"value\" + index] = document.getElementById(\"value\" + index);\n\
								//configValues[index] = 0.5;\n\
								window[\"value\" + index].innerHTML = configValues[index];\n\
								\n\
								document.getElementById(\"Up\" + index + \"Button\").onclick = function() {\n\
									configValues[index] += 0.5;\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(1);\n\
									uploadConfig();\n\
								}\n\
								document.getElementById(\"Down\" + index + \"Button\").onclick = function() {\n\
									configValues[index] -= 0.5;\n\
									configValues[index] = Math.max(lowerBound, configValues[index]);\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(1);\n\
									uploadConfig();\n\
								}\n\
							};\n\
						}\n\
						\n\
						function addFraction(name, index, lowerBound){\n\
							UIstring += \"\\\n\
								<tr>\\\n\
									<td><p>\" + name + \"=<br><span id=\\\"value\" + index + \"\\\"></span></p></td>\\\n\
									<td><button id=\\\"Down\" + index + \"Button\\\" class=\\\"button button1\\\">&#8681</button>\\\n\
									<button id=\\\"Up\" + index + \"Button\\\" class=\\\"button button1\\\">&#8679</button></td>\\\n\
									<td></td>\\\n\
								</tr>\\\n\
							\";\n\
							jsFunctions[index] = function(){\n\
								window[\"value\" + index] = document.getElementById(\"value\" + index);\n\
								//configValues[index] = 0.5;\n\
								window[\"value\" + index].innerHTML = configValues[index];\n\
								\n\
								document.getElementById(\"Up\" + index + \"Button\").onclick = function() {\n\
									configValues[index] += 0.05;\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(1);\n\
									uploadConfig();\n\
								}\n\
								document.getElementById(\"Down\" + index + \"Button\").onclick = function() {\n\
									configValues[index] -= 0.05;\n\
									configValues[index] = Math.max(lowerBound, configValues[index]);\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(1);\n\
									uploadConfig();\n\
								}\n\
							};\n\
						}\n\
						function addVariableConfig(name, index){\n\
							UIstring += \"\\\n\
								<tr>\\\n\
									<td><p>\" + name + \"=<br><span id=\\\"value\" + index + \"\\\"></span></p></td>\\\n\
									<td><button id=\\\"Down\" + index + \"Button\\\" class=\\\"button button1\\\">&#8681</button>\\\n\
									<button id=\\\"Up\" + index + \"Button\\\" class=\\\"button button1\\\">&#8679</button></td>\\\n\
									<td></td>\\\n\
								</tr>\\\n\
							\";\n\
							jsFunctions[index] = function(){\n\
								window[\"value\" + index] = document.getElementById(\"value\" + index);\n\
								window[\"value\" + index].innerHTML = configValues[index];\n\
								\n\
								document.getElementById(\"Up\" + index + \"Button\").onclick = function() {\n\
									configValues[index] += 0.0000001;\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(0);\n\
									uploadConfig();\n\
								}\n\
								document.getElementById(\"Down\" + index + \"Button\").onclick = function() {\n\
									configValues[index] -= 0.0000001;\n\
									window[\"value\" + index].innerHTML = configValues[index].toFixed(0);\n\
									uploadConfig();\n\
								}\n\
							};\n\
						}\n\
						\n\
						function updateConfigValuesInHTML(){\n\
							for(let i = 0; i < numConfigValues; i++){\n\
								if(document.getElementById(\"value\" + i) != null){\n\
									document.getElementById(\"value\" + i).innerHTML = configValues[i];\n\
									//console.log(document.getElementById(\"value\" + i));\n\
									//console.log(configValues[i]);\n\
								}\n\
							}\n\
						}\n\
						\n\
						downloadConfig();\n\
						\n\
						let tableBegin = \"\\\n\
								<table style=\\\"width:100%\\\">\\\n\
								<tbody>\\n\
						\";\n\
						let tableEnd = \"\\\n\
								</tbody></table>\\\n\
						\";\n\
						UIstring += \"<h3>Eight Flying Config</h3>\";\n\
						UIstring += tableBegin;\n\
						\n\
						addVariableConfig(\"BMP280 Calibration (Kite)\", 6)\n\
						addVariableConfig(\"BMP280 Calibration (Groundstation)\", " NUM_CONFIG_FLOAT_VARS_string " + 0)\n\
						addHeight(\"Sideways Flying Time (s)\", 12, 0.5)\n\
						addHeight(\"Turning Speed (deg/s)\", 13, 0.5)\n\
						addPIDConstant(\"Eights Yaw(Z) Compensation(P)\", 27);\n\
						addPIDConstant(\"Eights Yaw(Z) Damping(D)\", 28);\n\
						addPIDConstant(\"Eights Pitch(Y) Damping\", 29);\n\
						addDegrees(\"Eights Elevator Offset (deg)\", 30);\n\
						addDegrees(\"Transition Pitch(Y) Angle (deg)\", 31);\n\
						addDegrees(\"Desired Line Angle from Horizon (deg)\", 32);\n\
						addPIDConstant(\"Beta Clamp\", 33);\n\
						addPIDConstant(\"Beta Compensation\", 34);\n\
						addFraction(\"Neutral Beta Fraction\", 35, 0.05);\n\
						\n\
						UIstring += tableEnd;\n\
						UIstring += \"<h3>Hover Launch Config</h3>\";\n\
						UIstring += tableBegin;\n\
						\n\
						addPIDConstant(\"Hover Pitch(Y) Compensation\", 14);\n\
						addPIDConstant(\"Hover Pitch(Y) Damping\", 15);\n\
						addDegrees(\"Pitch(Y) Offset\", 16)\n\
						addPIDConstant(\"Hover Yaw(Z) Compensation\", 17);\n\
						addPIDConstant(\"Hover Yaw(Z) Damping\", 18);\n\
						addPIDConstant(\"Hover Roll(X) Damping\", 19);\n\
						addPIDConstant(\"Hover Height Compensation\", 20);\n\
						addPIDConstant(\"Hover Height Dampening\", 21);\n\
						\n\
						UIstring += tableEnd;\n\
						UIstring += \"<h3>Landing Config</h3>\";\n\
						UIstring += tableBegin;\n\
						\n\
						addDegrees(\"Brake Angle (deg)\", 22)\n\
						addPIDConstant(\"Landing Pitch(Y) Compensation\", 23);\n\
						addPIDConstant(\"Landing Pitch(Y) Damping\", 24);\n\
						addPIDConstant(\"Landing Roll(X) Compensation\", 25);\n\
						addPIDConstant(\"Dive Angle Compensation\", 36);\n\
						addHeight(\"Landing Approach Height (m)\", 26, -100)\n\
						\n\
						UIstring += tableEnd;\n\
						\n\
						document.getElementById(\"autogeneratedHTML\").innerHTML = UIstring;\n\
						\n\
						for(let i = 0; i < numConfigValues + numActuatorsWithoutConfig; i++){\n\
							if(typeof jsFunctions[i] === 'function'){\n\
								jsFunctions[i]();\n\
							}\n\
						}\n\
						\n\
						var time_index = 0;\n\
						function updateDebuggingDataGraph(){\n\
							for(let i = 0; i < 6; i++){\n\
								diagram.diagramLines[i].addPoint(time_index, debuggingData[i]);\n\
							}\n\
							//console.log(\"updating graph\");\n\
							//console.log(diagram.diagramLines)\n\
							if(!receiving_data_points_paused)time_index++;\n\
						}\n\
						\n\
						var animate = function(time) {\n\
							\n\
							if(ready_to_download_debugging_data){\n\
								downloadDebuggingData();\n\
						  		ready_to_download_debugging_data = false;\n\
						  	}\n\
		  					gl.disable(gl.DEPTH_TEST);\n\
							gl.depthFunc(gl.LEQUAL);\n\
							gl.clearColor(1.0, 1.0, 1.0, 1.0);\n\
							gl.clearDepth(1.0);\n\
							\n\
							gl.viewport(0.0, 0.0, canvas.width, canvas.height);\n\
							gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);\n\
							\n\
							//DRAW LINES\n\
							//diagram.diagramLines[0].addPoint(time_index, time_index*time_index*0.05);\n\
							//diagram.diagramLines[1].addPoint(time_index, Math.sin(time_index*0.05));\n\
							//diagram.diagramLines[2].addPoint(time_index, Math.cos(time_index*0.05));\n\
							//diagram.diagramLines[3].addPoint(time_index, Math.exp(time_index*0.05));\n\
							//diagram.diagramLines[4].addPoint(time_index, Math.cos(time_index*0.05)*Math.cos(time_index*0.05));\n\
							//diagram.diagramLines[5].addPoint(time_index, Math.sin(time_index*0.05)*Math.sin(time_index*0.05));\n\
							//if(!receiving_data_points_paused)time_index++;\n\
							diagram.render();\n\
							\n\
            				setTimeout(() => {  window.requestAnimationFrame(animate); }, 100);\n\
						 }\n\
						 animate(0);\n\
					</script>\n\
</body>\n\
</html>"
};

// ****** GET DEBUGGING DATA ******

static esp_err_t debugging_data_get_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Getting debuggind data");
	esp_err_t error;
	
    float float_values[6];
    (*debugging_data_callback)(float_values);
    
	char response2[6*20];
    sprintf(response2, "%f,%f,%f,%f,%f,%f", 
    	float_values[0],
    	float_values[1],
    	float_values[2],
    	float_values[3],
    	float_values[4],
    	float_values[5]
    );
    
    error = httpd_resp_send(req, response2, strlen(response2));
    return error;
}

static const httpd_uri_t kite_debugging_data_get_values = {
    .uri       = "/get_debugging_data",
    .method    = HTTP_GET,
    .handler   = debugging_data_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};


// ****** GET CONFIG ******

static esp_err_t config_get_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Getting config values (initialization)");
	esp_err_t error;
	
    float float_values[NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS];
    (*init_callback)(float_values);
    
	char response2[(NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS)*20];
    sprintf(response2, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", 
    	float_values[0],
    	float_values[1],
    	float_values[2],
    	float_values[3],
    	float_values[4],
    	float_values[5],
    	float_values[6]*1000000,
    	float_values[7],
    	float_values[8],
    	float_values[9],
    	float_values[10],
    	float_values[11],
    	float_values[12],
    	float_values[13],
    	float_values[14],
    	float_values[15],
    	float_values[16],
    	float_values[17],
    	float_values[18],
    	float_values[19],
    	float_values[20],
    	float_values[21],
    	float_values[22],
    	float_values[23],
    	float_values[24],
    	float_values[25],
    	float_values[26],
    	float_values[27],
    	float_values[28],
    	float_values[29],
    	float_values[30],
    	float_values[31],
    	float_values[32],
    	float_values[33],
    	float_values[34],
    	float_values[35],
    	float_values[36],
    	float_values[37],
    	float_values[38],
    	float_values[39],
    	float_values[40]
    );
    
    error = httpd_resp_send(req, response2, strlen(response2));
    return error;
}

static const httpd_uri_t kite_config_get_values = {
    .uri       = "/get_config",
    .method    = HTTP_GET,
    .handler   = config_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

int getIndexToNextNumber(char* string_arg, int current_index){
	while(string_arg[current_index] != 44){
    	current_index++;
    }
    current_index++;
    return current_index;
}

// ****** POST CONFIG ******

esp_err_t config_post_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Posting config to kite");
    char content[(NUM_CONFIG_FLOAT_VARS+NUM_GS_CONFIG_FLOAT_VARS)*20];

    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    
    if (ret <= 0) return ESP_FAIL;
    
    int string_position = 0;
    float config_float_values[NUM_CONFIG_FLOAT_VARS+NUM_GS_CONFIG_FLOAT_VARS];
    for(int i = 0; i < NUM_CONFIG_FLOAT_VARS+NUM_GS_CONFIG_FLOAT_VARS; i++){
    	config_float_values[i] = atof(content+string_position);
    	string_position = getIndexToNextNumber(content, string_position);
    }
	
	(*write_callback)(config_float_values);
	
    /* Send a simple response */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler structure for POST /uri */
httpd_uri_t config_post = {
    .uri      = "/uploadConfig",
    .method   = HTTP_POST,
    .handler  = config_post_handler,
    .user_ctx = NULL
};

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &kite_config_get_html);
        httpd_register_uri_handler(server, &kite_config_get_values);
        httpd_register_uri_handler(server, &kite_debugging_data_get_values);
        httpd_register_uri_handler(server, &config_post);
        #if CONFIG_EXAMPLE_BASIC_AUTH
        httpd_register_basic_auth(server);
        #endif
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

// ACCESS POINT FUNCTIONALITY

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

// init wifi on the esp
// register callbacks
void network_setup_configuring(void (*write_callback_arg)(float*), void (*init_callback_arg)(float*), void (*debugging_data_callback_arg)(float*))
{
	write_callback = write_callback_arg;
	init_callback = init_callback_arg;
	debugging_data_callback = debugging_data_callback_arg;
	static httpd_handle_t server = NULL;
	
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    
    
    ESP_ERROR_CHECK(esp_netif_init());
    
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
    //ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
    
    /* Start the server for the first time */
    //server = start_webserver(); // will be started in connect_handler
    
}
