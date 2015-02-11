#version 120

uniform sampler2D colorTexture;
varying vec2 fragTexCoords;

void main()
{
	vec4 c = texture2D(colorTexture, fragTexCoords);
	gl_FragColor = vec4(c.rgb, 1.0);
}
