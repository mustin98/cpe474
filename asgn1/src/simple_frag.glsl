#version 120

struct Material {
   vec3 dColor;
   vec3 sColor;
   vec3 aColor;
   float shine;
};

uniform mat4 MV;
uniform mat4 V;
uniform vec3 lightPos;
uniform vec3 camPos;
uniform Material mat;

varying vec3 fragNor;
varying vec4 fragPos;

void main() {
   vec4 normal, light, view, reflection;
   vec3 D, S, A;
   float nDotL, vDotR;

   normal = normalize(vec4(fragNor, 0.0));
   view = normalize(MV * vec4(camPos, 1) - fragPos);
   light = normalize(V * vec4(lightPos, 1.0) - fragPos);
   nDotL = clamp(dot(light, normal), 0.0, 1.0);
   reflection = reflect(-light, normal);
   vDotR = clamp(dot(view, reflection), 0.0, 1.0);

   D = nDotL * mat.dColor;
   S = pow(vDotR, mat.shine) * mat.sColor;
   A = mat.aColor;

   gl_FragColor = vec4(D + S + A, 1);
}