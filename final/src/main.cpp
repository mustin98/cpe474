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

Program prog;
Program progTex;
Camera camera;
Shape rocket;
ShapeObj ground;
Texture groundTex;
Eigen::Vector3f light = Eigen::Vector3f(0, 0, 2);

void loadScene() {
	t = 0.0f;
	keyToggles['c'] = true;
	rocket.addObj("../models/roket.obj", "../models");

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
	
	Eigen::Vector3f x_axis;
	Eigen::Vector3f y_axis;
	Eigen::Vector3f z_axis;
	x_axis << 1, 0, 0;
	y_axis << 0, 1, 0;
	z_axis << 0, 0, 1;

	ground.init();
	groundTex.init();
	rocket.init();
	rocket.addCP(Eigen::Vector3f(0, 0, 0), Eigen::AngleAxisf(90.0f/180.0f*M_PI, y_axis));
	rocket.addCP(Eigen::Vector3f(5, 0, 0), Eigen::AngleAxisf(180.0f/180.0f*M_PI, y_axis));
	rocket.addCP(Eigen::Vector3f(3, 1, 0), Eigen::AngleAxisf(270.0f/180.0f*M_PI, y_axis));
	rocket.addCP(Eigen::Vector3f(2.5, 5, 0), Eigen::AngleAxisf(45.0f/180.0f*M_PI, x_axis));
	rocket.addCP(Eigen::Vector3f(-1, -2, -2), Eigen::AngleAxisf(270.0f/180.0f*M_PI, y_axis));
	rocket.addCP(Eigen::Vector3f(-2, 0, -5), Eigen::AngleAxisf(180.0f/180.0f*M_PI, y_axis));
	rocket.addCP(Eigen::Vector3f(0, 1, -2), Eigen::AngleAxisf(90.0f/180.0f*M_PI, y_axis));

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
	if(keyToggles[' ']) {
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
	
	// Draw frame
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glEnd();
	glLineWidth(1);
	
	if (keyToggles['k']) {
		rocket.drawSpline();
	}

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

	/*
	if (keyToggles['k']) {
		rocket.drawKeyFrames(prog, MV);
	}
	rocket.draw(prog, MV, t);
	*/
	
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
		case 't':
			t = 0.0f;
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
