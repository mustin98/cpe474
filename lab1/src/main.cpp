#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <iostream>
#include <string.h>
#include <vector>
#include <cmath>
#include "GLSL.h"
#include "Program.h"

using namespace std;

float width;
float height;

bool keyToggles[256] = {false};
float t = 0.0f;
float h = 0.01f;

Program prog;

float t0_disp = 0.0f;
float t_disp = 0.0f;

void loadScene()
{
	t = 0.0f;
	h = 0.000001f;
	
	prog.setShaderNames("lab01_vert.glsl", "lab01_frag.glsl");
}

void initGL()
{
	//////////////////////////////////////////////////////
	// Initialize GL for the whole scene
	//////////////////////////////////////////////////////
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//////////////////////////////////////////////////////
	// Intialize the shaders
	//////////////////////////////////////////////////////
	
	prog.init();
	prog.addUniform("P");
	prog.addUniform("MV");
	
	//////////////////////////////////////////////////////
	// Final check for errors
	//////////////////////////////////////////////////////
	GLSL::checkVersion();
}

void reshapeGL(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

void setIdentity(float *M)
{
	memset(M, 0, 16*sizeof(float));
	M[0] = M[5] = M[10] = M[15] = 1.0f;
}

void setOrtho2D(float *M, float left, float right, float bottom, float top, float zNear, float zFar)
{
	memset(M, 0, 16*sizeof(float));
	M[0] = 2.0f / (right - left);
	M[5] = 2.0f / (top - bottom);
	M[10] = -2.0f / (zFar - zNear);
	M[12] = - (right + left) / (right - left);
	M[13] = - (top + bottom) / (top - bottom);
	M[14] = - (zFar + zNear) / (zFar - zNear);
	M[15] = 1.0f;
}

void drawGL()
{
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Enable backface culling
	if(keyToggles['c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles['l']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Projection matrix
	float P[16];
	setOrtho2D(P, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
	
	// Modelview matrix
	float MV[16];
	setIdentity(MV);
	
	// Bind the program
	prog.bind();
	glUniformMatrix4fv(prog.getUniform("P"), 1, GL_FALSE, P);
	glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV);
	
	// Old-school OpenGL
	// OK for ang shape consisting of a few (<1000) vertices
	
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_STRIP);
	
	double a = 5, b = 4, d = M_PI/2;
	int samples = 1000;

	for (double ang = 0.0; ang <= 2 * M_PI + M_PI / samples + t; ang += 2 * M_PI / samples) {
		float yellow = ang/(2*M_PI) + t;
		
		if (yellow >1.0) {
			yellow -= 1.0;
		}
		glColor3f(1 - yellow, yellow, yellow);
		glVertex3f(sin(a*(ang) + d), sin(b*(ang)), 0.0);
	}
	glEnd();
	
	// Unbind the program
	prog.unbind();

	// Double buffer
	glutSwapBuffers();
}

void keyboardGL(unsigned char key, int x, int y)
{
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 27:
			// ESCAPE
			exit(0);
			break;
	}
}

void idleGL()
{
	if(keyToggles[' ']) {
		t += h;
		
		if (t > 1.0) {
			t -= 1.0; 
		}
	}
	
	// Display every 60 Hz
	t_disp = glutGet(GLUT_ELAPSED_TIME);
	if(t_disp - t0_disp > 1000.0f/60.0f) {
		// Ask for refresh
		glutPostRedisplay();
		t0_disp = t_disp;
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(400, 400);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Todd=>Justin");
	glutKeyboardFunc(keyboardGL);
	glutReshapeFunc(reshapeGL);
	glutDisplayFunc(drawGL);
	glutIdleFunc(idleGL);
	loadScene();
	initGL();
	glutMainLoop();
	return 0;
}
