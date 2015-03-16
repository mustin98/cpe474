#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "ShapeObj.h"
#include "Shape.h"
#include "Texture.h"

using namespace std;

bool keyToggles[256] = {false};

float t = 0.0f;
float tPrev = 0.0f;
int width = 1;
int height = 1;
bool inSession = false;

Program prog;
Program progTex;
Camera camera;
Shape rocket;
ShapeObj ground;
Texture groundTex;
Eigen::Vector3f light = Eigen::Vector3f(0, 5, -4);

void loadScene() {
	t = 0.0f;
	keyToggles['c'] = true;
	rocket.addObj("../models/roket.obj", "../models", Eigen::Vector3f(0,0,0), Eigen::Vector3f(0,0,1));

	ground.load("../models/ground.obj");
	groundTex.setFilename("../models/grass_tex.jpg");

	prog.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
	progTex.setShaderNames("tex_vert.glsl", "tex_frag.glsl");
}

void initGL() {
	//////////////////////////////////////////////////////
	// Initialize GL for the whole scene
	//////////////////////////////////////////////////////
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	//////////////////////////////////////////////////////
	// Intialize the shapes
	//////////////////////////////////////////////////////
	
	ground.init();
	groundTex.init();
	rocket.init();
	// Add ControlBoxes (Position, Dimensions, first/last)
	rocket.addCB(Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,10), true);
	rocket.addCB(Eigen::Vector3f(1,0,0), Eigen::Vector3f(1,1,10), false);
	rocket.addCB(Eigen::Vector3f(1,1,0), Eigen::Vector3f(1,1,10), false);
	rocket.addCB(Eigen::Vector3f(2,2,0), Eigen::Vector3f(1,1,10), false);
	rocket.addCB(Eigen::Vector3f(3,5,0), Eigen::Vector3f(1,1,10), false);
	rocket.addCB(Eigen::Vector3f(4,8,0), Eigen::Vector3f(1,1,10), false);
	rocket.addCB(Eigen::Vector3f(4,3,0), Eigen::Vector3f(1,1,10), false);
	rocket.addCB(Eigen::Vector3f(5,0,0), Eigen::Vector3f(1,1,10), false);
	rocket.addCB(Eigen::Vector3f(6,6,0), Eigen::Vector3f(1,1,10), true);

	// obj has huge coordinates so we need to rescale and center it
	rocket.center(Eigen::Vector3f(0, 0, 260.2751));
	rocket.rescale(0.001);

	
	//////////////////////////////////////////////////////
	// Intialize the shaders
	//////////////////////////////////////////////////////
	
	prog.init();
	prog.addUniform("P");
	prog.addUniform("MV");
	prog.addUniform("V");
	prog.addUniform("lightPos");
	prog.addUniform("camPos");
	prog.addUniform("dif");
	prog.addUniform("spec");
	prog.addUniform("amb");
	prog.addUniform("shine");
	prog.addAttribute("vertPos");
	prog.addAttribute("vertNor");

	progTex.init();
	progTex.addAttribute("vertPos");
	progTex.addAttribute("vertTex");
	progTex.addUniform("P");
	progTex.addUniform("MV");
	progTex.addUniform("colorTexture");
	
	//////////////////////////////////////////////////////
	// Final check for errors
	//////////////////////////////////////////////////////
	GLSL::checkVersion();
}

void reshapeGL(int w, int h) {
	// Set view size
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	camera.setAspect((float)width/(float)height);
}

void drawGL() {
	// Elapsed time
	float tCurr = 0.001f*glutGet(GLUT_ELAPSED_TIME); // in seconds
	if(inSession) {
		t += (tCurr - tPrev);
	}
	tPrev = tCurr;
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
	
	//////////////////////////////////////////////////////
	// Create matrix stacks
	//////////////////////////////////////////////////////
	
	MatrixStack P, MV;

	// Apply camera transforms
	P.pushMatrix();
	camera.applyProjectionMatrix(&P);
	MV.pushMatrix();
	camera.applyViewMatrix(&MV);

	//////////////////////////////////////////////////////
	// Draw origin frame using old-style OpenGL
	// (before binding the program)
	//////////////////////////////////////////////////////
	
	// Setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(P.topMatrix().data());
	
	// Setup the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(MV.topMatrix().data());

	if (keyToggles['k']) {
		rocket.drawSpline();
	}
	rocket.drawCPs();

	// Pop modelview matrix
	glPopMatrix();
	// Pop projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	// Draw string
	P.pushMatrix();
	P.ortho(0, width, 0, height, -1, 1);
	glPushMatrix();
	glLoadMatrixf(P.topMatrix().data());
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRasterPos2f(5.0f, 5.0f);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	P.popMatrix();

	//////////////////////////////////////////////////////
	// Now draw the shape using modern OpenGL
	//////////////////////////////////////////////////////
	
	// Bind the program
	prog.bind();

	// Send projection matrix (same for all objects)
	glUniformMatrix4fv(prog.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(prog.getUniform("V"), 1, GL_FALSE, MV.topMatrix().data());
	MV.pushMatrix();
	glUniform3fv(prog.getUniform("lightPos"), 1, light.data());
	glUniform3fv(prog.getUniform("camPos"), 1, camera.translations.data());

	rocket.draw(prog, MV, t);
	
	// Unbind the program
	prog.unbind();

	// Draw textured shapes
	progTex.bind();
	groundTex.bind(progTex.getUniform("colorTexture"), 0);
	glUniformMatrix4fv(progTex.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progTex.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	ground.draw(progTex.getAttribute("vertPos"), -1, progTex.getAttribute("vertTex"));
	groundTex.unbind(0);
	progTex.unbind();

	//////////////////////////////////////////////////////
	// Cleanup
	//////////////////////////////////////////////////////
	
	// Pop stacks
	MV.popMatrix();
	P.popMatrix();
	
	// Swap buffer
	glutSwapBuffers();
}

void mouseGL(int button, int state, int x, int y) {
	int modifier = glutGetModifiers();
	bool shift = modifier & GLUT_ACTIVE_SHIFT;
	bool ctrl  = modifier & GLUT_ACTIVE_CTRL;
	bool alt   = modifier & GLUT_ACTIVE_ALT;
	camera.mouseClicked(x, y, shift, ctrl, alt);
}

void mouseMotionGL(int x, int y) {
	camera.mouseMoved(x, y);
}

void keyboardGL(unsigned char key, int x, int y) {
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 27:
			// ESCAPE
			exit(0);
			break;
		case ' ': // begin
			inSession = true;
			break;
		case 'r': // restart
			t = 0.0f;
			inSession = false;
			break;
		case '.': // next ControlBox
			rocket.switchCB(1);
			break;
		case ',': // prev ControlBox
			rocket.switchCB(-1);
			break;
		case 'm':
			if(!inSession) {
				rocket.xMove(0.1);
			}
			break;
		case 'n':
			if(!inSession) {
				rocket.xMove(-0.1);
			}
			break;
		case 'y':
			if(!inSession) {
				rocket.yMove(0.1);
			}
			break;
		case 'h':
			if(!inSession) {
				rocket.yMove(-0.1);
			}
			break;
		case 'j':
			if(!inSession) {
				rocket.zMove(0.1);
			}
			break;
		case 'u':
			if(!inSession) {
				rocket.zMove(-0.1);
			}
			break;
	}
}

void timerGL(int value) {
	glutPostRedisplay();
	glutTimerFunc(20, timerGL, 0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(600, 600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Final Project - Justin Fujikawa");
	glutMouseFunc(mouseGL);
	glutMotionFunc(mouseMotionGL);
	glutKeyboardFunc(keyboardGL);
	glutReshapeFunc(reshapeGL);
	glutDisplayFunc(drawGL);
	glutTimerFunc(20, timerGL, 0);
	loadScene();
	initGL();
	glutMainLoop();
	return 0;
}
