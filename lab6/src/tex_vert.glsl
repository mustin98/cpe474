#version 120

attribute vec2 vertLocalPos;
attribute vec2 vertTex;
attribute float tileIndex;

uniform mat4 P;
uniform mat4 MV;
uniform vec2 cps[25];

varying vec2 fragTexCoords;

void main()
{
   float u = vertLocalPos.x;
   float v = vertLocalPos.y;
   vec2 p = (1-v) * ((1-u)*cps[int(tileIndex)]+u*cps[int(tileIndex) + 1]) + v * ((1-u)*cps[int(tileIndex) + 5]+u*cps[int(tileIndex) + 5 + 1]);

	gl_Position = P * MV * vec4(p, 0, 1);
	fragTexCoords = vertTex;
}
