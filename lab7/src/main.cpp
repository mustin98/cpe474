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
#include <Eigen/Dense>
#include "GLSL.h"
#include "Program.h"
#include "Camera.h"
#include "MatrixStack.h"

using namespace std;

int width = 1;
int height = 1;
bool keyToggles[256] = {false};
Camera camera;
Program progSimple;
Eigen::Vector2f mouse;
Eigen::Vector4f c0;
Eigen::Vector4f c1;
float xmid;

void loadScene()
{
	camera.setTranslations(Eigen::Vector3f(0.5f, 0.5f, 1.0f));
	progSimple.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
	
	//
	// Compute the coefficients here
	//
	xmid = 0.4f;
	Eigen::MatrixXf A(8,8);
	Eigen::VectorXf b(8);
	A.setZero();
	b.setZero();
	A.block<1,4>(0,0) << 0.0f, 0.0f, 0.0f, 1.0f;
	b(0) = 0.0f;
	
	A.block<1,4>(1,0) << 0.0f, 0.0f, 0.0f, 0.0f;
	b(1) = 0.0f;
	
	A.block<1,4>(2,0) << 0.4f*0.4f*0.4f, 0.4f*0.4f, 0.4f, 1.0f;
	A.block<1,4>(2,4) << -0.4f*-0.4f*-0.4f, -0.4f*-0.4f, -0.4f, -1.0f;
	b(2) = 0.0f;

	A.block<1,4>(3,0) << 3*0.4f*0.4f*0.4f, 2*0.4f*0.4f, 0.4f, 1.0f;
	A.block<1,4>(3,4) << -3*.04f*0.04f*0.04f, -2*0.04f*0.4f, -0.4f, 0.0f;
	b(3) = 0.0f;
	
	A.block<1,4>(4,4) << 0.5f*0.5f*0.5f, 0.5f*0.5f, 0.5f, 1.0f;
	b(4) = 0.2f;
	
	A.block<1,4>(5,4) << 3*0.5f*0.5f, 2*0.5f, 0.5f, 0.0f;
	b(5) = 0.0f;
	
	A.block<1,4>(6,4) << 1.0f, 1.0f, 1.0f, 1.0f;
	b(6) = 1.0f;
	
	A.block<1,4>(7,4) << 3*0.5f*0.5f, 2*0.5f, 0.5f, 0.0f;
	b(7) = 0.0f;
	cout << A << endl;
	cout << b << endl;
	Eigen::VectorXf x = A.colPivHouseholderQr().solve(b);
	c0 = x.segment<4>(0);
	c1 = x.segment<4>(4);
}

void initGL()
{
	// Set background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Disable z-buffer test
	glDisable(GL_DEPTH_TEST);
	
	progSimple.init();
	progSimple.addUniform("P");
	progSimple.addUniform("MV");
	
	GLSL::checkVersion();
}

void reshapeGL(int w, int h)
{
	// Set view size
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	float aspect = w/(float)h;
	float s = 0.6f;
	camera.setOrtho(-s*aspect, s*aspect, -s, s, 0.1f, 10.0f);
}

void drawGL()
{
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Apply camera transforms
	MatrixStack P, MV;
	camera.applyProjectionMatrix(&P);
	camera.applyViewMatrix(&MV);
	
	progSimple.bind();
	glUniformMatrix4fv(progSimple.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progSimple.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	// Draw grid
	int gridSize = 5;
	glLineWidth(1.0f);
	glColor3f(0.2f, 0.2f, 0.2f);
	glBegin(GL_LINES);
	for(int i = 1; i < gridSize; ++i) {
		float x = i / (float)gridSize;
		glVertex2f(x, 0.0f);
		glVertex2f(x, 1.0f);
	}
	for(int i = 1; i < gridSize; ++i) {
		float y = i / (float)gridSize;
		glVertex2f(0.0f, y);
		glVertex2f(1.0f, y);
	}
	glEnd();
	glLineWidth(2.0f);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_LINE_LOOP);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(1.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (float x = 0.0f; x < 1.0f; x += 0.01) {
		if (x <= xmid) {
			glColor3f(1.0,0,0);
			glVertex2f(x, c0(0) * x*x*x + c0(1)*x*x+c0(2)*x+c0(3));
		} else {
			glColor3f(0,1.0,0);
			glVertex2f(x, c1(0) * x*x*x + c1(1)*x*x+c1(2)*x+c1(3));
		}
	}
	glEnd();

	glPointSize(5);
	glBegin(GL_POINTS);
	float slope, y;
	float x = std::max(0.0f, std::min(mouse(0), 1.0f));
	if (x <= xmid && x >= 0.0f) {
		y = c0(0) * x*x*x + c0(1)*x*x+c0(2)*x+c0(3);
		slope = 3*c0(0) *x*x + 2*c0(1)*x+c0(2);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(x, y);
	} else if (x > xmid && x <= 1.0f) {
		y = c1(0) * x*x*x + c1(1)*x*x+c1(2)*x+c1(3);
		slope = 3*c1(0) *x*x + 2*c1(1)*x+c1(2);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(x, c1(0) * x*x*x + c1(1)*x*x+c1(2)*x+c1(3));
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex2f(x-0.1, y-0.1*slope);
	glVertex2f(x+0.1, y+0.1*slope);
	glEnd();
	//
	// Draw cubics here
	//
	progSimple.unbind();

	// Double buffer
	glutSwapBuffers();
}

Eigen::Vector2f window2world(int x, int y)
{
	// Convert from window coords to world coords
	// (Assumes orthographic projection)
	Eigen::Vector4f p;
	// Inverse viewing transform
	p(0) = x / (float)width;
	p(1) = (height - y) / (float)height;
	p(0) = 2.0f * (p(0) - 0.5f);
	p(1) = 2.0f * (p(1) - 0.5f);
	p(2) = 0.0f;
	p(3) = 1.0f;
	// Inverse model-view-projection transform
	MatrixStack P, MV;
	camera.applyProjectionMatrix(&P);
	camera.applyViewMatrix(&MV);
	p = (P.topMatrix() * MV.topMatrix()).inverse() * p;
	return p.segment<2>(0);
}

void mousePassiveMotionGL(int x, int y)
{
	mouse = window2world(x, y);
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
	glutCreateWindow("Your Name");
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
