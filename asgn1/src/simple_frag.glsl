#version 120

uniform mat4 MV;
varying vec3 fragNor;
varying vec4 vPos;

void main() {
   vec4 normal, light, view, reflection;
   vec3 D, S, A;
   float nDotL, vDotR;

   normal = normalize(vec4(fragNor.x, fragNor.y, fragNor.z, 0));
   view = normalize(MV * vec4(0, 0, 0, 1) - MV*vPos);
   light = normalize(MV * vec4(0, 1, -1, 0));
   nDotL = clamp(dot(light, normal), 0.0, 1.0);
   reflection = reflect(-light, normal);
   vDotR = clamp(dot(view, reflection), 0.0, 1.0);

   D = nDotL * vec3(.6, .2, .2);
   S = pow(vDotR, 100) * vec3(.5, .2, .2);
   A = vec3(.2, .2, .2);

   gl_FragColor = vec4(D + S + A, 1);
}
