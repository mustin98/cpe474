#version 120

attribute vec4 vertPos;
attribute vec2 vertTex;
uniform mat4 P;
uniform mat4 MV;
varying vec2 fragTexCoords;

void main()
{
	gl_Position = P * MV * vertPos;
	fragTexCoords = vertTex;
}
