#version 120

attribute vec4 vertPos;
attribute vec3 vertNor;
attribute float numBones;
attribute vec4 weights0;
attribute vec4 weights1;
attribute vec4 weights2;
attribute vec4 weights3;
attribute vec4 bones0;
attribute vec4 bones1;
attribute vec4 bones2;
attribute vec4 bones3;

uniform mat4 M[18];
uniform mat4 M0[18];

uniform mat4 P;
uniform mat4 MV;

varying vec4 fragPos;
varying vec3 fragNor;

void main() {
   int b2;      // boneNum
   int j;       // bone
   float w;     // skinning weight
   vec4 x0 = vertPos;          // initial vert pos
   vec4 n0 = vec4(vertNor, 0); // initial norm vec
   vec4 x = vec4(0); // new vert pos
   vec4 n = vec4(0); // new norm vec

   for (int b = 0; b < int(numBones); b++) {
      if (b < 4) {
         b2 = b - 0;
         j = int(bones0[b2]);
         w = weights0[b2];

         x += w * (M[j] * (M0[j] * x0));
         n += w * (M[j] * (M0[j] * n0));
      }
      else if (b < 8) {
         b2 = b - 4;
         j = int(bones1[b2]);
         w = weights1[b2];

         x += w * (M[j] * (M0[j] * x0));
         n += w * (M[j] * (M0[j] * n0));
      }
      else if (b < 12) {
         b2 = b - 8;
         j = int(bones2[b2]);
         w = weights2[b2];

         x += w * (M[j] * (M0[j] * x0));
         n += w * (M[j] * (M0[j] * n0));
      } 
      else {
         b2 = b - 12;
         j = int(bones3[b2]);
         w = weights3[b2];

         x += w * (M[j] * (M0[j] * x0));
         n += w * (M[j] * (M0[j] * n0));
      }
   }

   fragPos = MV * x;
   fragNor = (MV * vec4(n.x, n.y, n.z, 0.0)).xyz;
	gl_Position = P * MV * x;
}
