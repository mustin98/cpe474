#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <memory>
#include <Eigen/Dense>
#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"
#include "ShapeObj.h"
#include "Texture.h"
#include "Grid.h"

using namespace std;

int width = 1;
int height = 1;
bool keyToggles[256] = {false};
Camera camera;
ShapeObj shape;
Texture texture;
Program progSimple;
Program progTex;
Grid grid;

void loadScene()
{
	shape.load("man.obj");
	texture.setFilename("wood_tex.jpg");
	progSimple.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
	progTex.setShaderNames("tex_vert.glsl", "tex_frag.glsl");
	
	// Grid of control points
	grid.setSize(5, 5);
	// Transform shape to local coordinates
	shape.setGrid(&grid);
	shape.toLocal();
}

void initGL()
{
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	shape.init();
	texture.init();
	progSimple.init();
	progSimple.addUniform("P");
	progSimple.addUniform("MV");
	progTex.init();
	progTex.addAttribute("vertPos");
	progTex.addAttribute("vertTex");
	progTex.addUniform("P");
	progTex.addUniform("MV");
	progTex.addUniform("colorTexture");
	
	GLSL::checkVersion();
}

void reshapeGL(int w, int h)
{
	// Set view size
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	float aspect = w/(float)h;
	float s = 1.5f;
	camera.setOrtho(-s*aspect, s*aspect, -s, s, 0.1f, 10.0f);
}

void drawGL()
{
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles['c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles['z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Apply camera transforms
	MatrixStack P, MV;
	camera.applyProjectionMatrix(&P);
	camera.applyViewMatrix(&MV);

	// Draw control points
	progSimple.bind();
	glUniformMatrix4fv(progSimple.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progSimple.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	grid.draw();
	progSimple.unbind();
	
	// Draw textured shape
	progTex.bind();
	texture.bind(progTex.getUniform("colorTexture"), 0);
	glUniformMatrix4fv(progTex.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progTex.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	shape.toWorld();
	shape.draw(progTex.getAttribute("vertPos"), progTex.getAttribute("vertTex"));
	texture.unbind(0);
	progTex.unbind();
	
	// Double buffer
	glutSwapBuffers();
}

Eigen::Vector2f window2world(int x, int y)
{
	// Convert from window coords to world coords
	// (Assumes orthographic projection)
	Eigen::Vector4f p;
	p(0) = x / (float)width;
	p(1) = (height - y) / (float)height;
	p(0) = 2.0f * (p(0) - 0.5f);
	p(1) = 2.0f * (p(1) - 0.5f);
	p(2) = 0.0f;
	p(3) = 1.0f;
	MatrixStack P_, MV_;
	camera.applyProjectionMatrix(&P_);
	camera.applyViewMatrix(&MV_);
	Eigen::Matrix4f P, MV;
	P = P_.topMatrix();
	MV = MV_.topMatrix();
	Eigen::Matrix4f mat = P * MV;
	p = mat.inverse() * p;
	return p.segment<2>(0);
}

void mouseGL(int button, int state, int x, int y)
{
	grid.moveCP(window2world(x, y));
}

void mouseMotionGL(int x, int y)
{
	grid.moveCP(window2world(x, y));
}

void mousePassiveMotionGL(int x, int y)
{
	grid.findClosest(window2world(x, y));
}

void keyboardGL(unsigned char key, int x, int y)
{
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 27:
			// ESCAPE
			exit(0);
			break;
		case 'r':
			grid.reset();
			break;
		case 's':
			grid.save("cps.txt");
			break;
		case 'l':
			grid.load("cps.txt");
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
	glutCreateWindow("Justin Fujikawa");
	glutMouseFunc(mouseGL);
	glutMotionFunc(mouseMotionGL);
	glutPassiveMotionFunc(mousePassiveMotionGL);
	glutKeyboardFunc(keyboardGL);
	glutReshapeFunc(reshapeGL);
	glutDisplayFunc(drawGL);
	glutTimerFunc(20, timerGL, 0);
	loadScene();
	initGL();
	glutMainLoop();
	return 0;
}
