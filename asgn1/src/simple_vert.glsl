#version 120

attribute vec4 vertPos;
attribute vec3 vertNor;

uniform mat4 P;
uniform mat4 MV;

varying vec4 fragPos;
varying vec3 fragNor;

void main() {
   fragPos = MV * vertPos;
   fragNor = (MV * vec4(vertNor, 0.0)).xyz;
	gl_Position = P * MV * vertPos;
}
