#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#ifdef _WIN32
#define GLFW_INCLUDE_GLCOREARB
#include <GL/glew.h>
#include <cstdlib>
#include <glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"

using namespace std;

int width = 1;
int height = 1;
bool keyToggles[256] = {false};
Camera camera;
Program progSimple;
Program progTex;
Shape shape;
Texture texture;
Eigen::Vector2f mouse(-1.0f, -1.0f);
int modifiers;

void loadScene()
{
	camera.setTranslations(Eigen::Vector3f(0.0f, 0.0f, 5.0f));
	shape.loadMesh("link.obj");
	texture.setFilename("metal_texture_15_by_wojtar_stock.jpg");
	progSimple.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
	progTex.setShaderNames("tex_vert.glsl", "tex_frag.glsl");
}

void initGL()
{
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Disable z-buffer test
	glEnable(GL_DEPTH_TEST);
	// Antialiasing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	
	shape.init();
	
	texture.init();
	
	progSimple.init();
	progSimple.addUniform("P");
	progSimple.addUniform("MV");
	
	progTex.init();
	progTex.addUniform("P");
	progTex.addUniform("MV");
	progTex.addAttribute("vertPos");
	progTex.addAttribute("vertNor");
	progTex.addAttribute("vertTex");
	progTex.addUniform("texture0");
	
	GLSL::checkVersion();
}

void reshapeGL(int w, int h)
{
	// Set view size
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	float aspect = w/(float)h;
	camera.setPerspective(aspect, 45.0f/180.0f*M_PI, 0.1f, 100.0f);
}

void drawGL()
{
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Apply camera transforms
	MatrixStack P, MV;
	camera.applyProjectionMatrix(&P);
	camera.applyViewMatrix(&MV);
	
	// Draw grid
	progSimple.bind();
	glUniformMatrix4fv(progSimple.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progSimple.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	glLineWidth(2.0f);
	float x0 = -2.0f;
	float x1 = 2.0f;
	float y0 = -2.0f;
	float y1 = 2.0f;
	glColor3f(0.2f, 0.2f, 0.2f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(x0, y0);
	glVertex2f(x1, y0);
	glVertex2f(x1, y1);
	glVertex2f(x0, y1);
	glEnd();
	int gridSize = 8;
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	for(int i = 1; i < gridSize; ++i) {
		if(i == gridSize/2) {
			glColor3f(0.2f, 0.2f, 0.2f);
		} else {
			glColor3f(0.8f, 0.8f, 0.8f);
		}
		float x = x0 + i / (float)gridSize * (x1 - x0);
		glVertex2f(x, y0);
		glVertex2f(x, y1);
	}
	for(int i = 1; i < gridSize; ++i) {
		if(i == gridSize/2) {
			glColor3f(0.2f, 0.2f, 0.2f);
		} else {
			glColor3f(0.8f, 0.8f, 0.8f);
		}
		float y = y0 + i / (float)gridSize * (y1 - y0);
		glVertex2f(x0, y);
		glVertex2f(x1, y);
	}
	glEnd();
	progSimple.unbind();
	
	// Draw shape
	progTex.bind();
	texture.bind(progTex.getUniform("texture0"), 0);
	glUniformMatrix4fv(progTex.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	MV.pushMatrix();
	glUniformMatrix4fv(progTex.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	shape.draw(&progTex);
	MV.popMatrix();
	texture.unbind();
	progTex.unbind();

	// Double buffer
	glutSwapBuffers();
}

void mouseGL(int button, int state, int x, int y)
{
	modifiers = glutGetModifiers();
	bool shift = modifiers & GLUT_ACTIVE_SHIFT;
	bool ctrl  = modifiers & GLUT_ACTIVE_CTRL;
	bool alt   = modifiers & GLUT_ACTIVE_ALT;
	camera.mouseClicked(x, y, shift, ctrl, alt);
	
	if(modifiers & GLUT_ACTIVE_ALT) {
		mouse(0) = x;
		mouse(1) = y;
	}
}

void mouseMotionGL(int x, int y)
{
	camera.mouseMoved(x, y);
	
	if(modifiers & GLUT_ACTIVE_ALT) {
		if(mouse(0) < 0.0f && mouse(1) < 0.0f) {
			mouse(0) = x;
			mouse(1) = y;
		}
		float s = 0.01f;
		float dx = s*(x - mouse(0));
		float dy = s*(y - mouse(1));
		//
		// Use dx and dy to change the joint angles
		//
		cout << dx << " " << dy << endl;
		mouse(0) = x;
		mouse(1) = y;
	}
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

void timerGL(int value)
{
	glutPostRedisplay();
	glutTimerFunc(20, timerGL, 0);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(400, 400);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Justin Todd");
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
