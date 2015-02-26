#version 120

varying vec3 fragPos; // in camera space
varying vec3 fragNor; // in camera space

void main()
{
	vec3 lightPos = vec3(0.0, 0.0, 0.0);
	vec3 n = normalize(fragNor);
	vec3 l = normalize(lightPos - fragPos);
	vec3 v = -normalize(fragPos);
	vec3 h = normalize(l + v);
	vec3 kd = vec3(1.0, 1.0, 1.0);
	vec3 diffuse = max(dot(l, n), 0.0) * kd;
	vec3 color = diffuse;
	gl_FragColor = vec4(color, 1.0);
}
