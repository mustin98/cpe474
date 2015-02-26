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
#include "Particle.h"
#include "TimeStepper.h"
#include "HUD.h"

using namespace std;

int width = 1;
int height = 1;
bool keyToggles[256] = {false};
Camera camera;
Program prog;
Program progSimple;
Shape shape;
vector<Particle*> particles;
TimeStepper stepper;
float tDisp;
float tDispPrev;
float fps;

void loadScene()
{
	tDisp = 0.0f;
	tDispPrev = 0.0f;
	fps = 0.0f;
	keyToggles['c'] = true;

	camera.setTranslations(Eigen::Vector3f(0.0f, 0.0f, 20.0f));
	progSimple.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
	prog.setShaderNames("phong_vert.glsl", "phong_frag.glsl");
	prog.setVerbose(false);
	shape.loadMesh("sphere2.obj");
	
	stepper.h = 1e-2;
	
	Particle *p0 = new Particle(&shape);
	Particle *p1 = new Particle(&shape);
	
	particles.push_back(p0);
	particles.push_back(p1);
	
	p0->r = 0.25f;
	p0->m = 1.0;
	p0->x << 3.0, 0.0, 0.0;
	p0->v << 0.0, 5.0, 0.0;
	
	p1->r = 0.25f;
	p1->m = 1.0;
	p1->x << -3.0, 0.0, 0.0;
	p1->v << 0.0, -5.0, 0.0;
	
	for(int i = 0; i < (int)particles.size(); ++i) {
		particles[i]->tare();
	}
}

void initGL()
{
	// Set background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);
	
	progSimple.init();
	progSimple.addUniform("P");
	progSimple.addUniform("MV");
	
	prog.init();
	prog.addUniform("P");
	prog.addUniform("MV");
	prog.addAttribute("vertPos");
	prog.addAttribute("vertNor");
	prog.addAttribute("vertTex");
	
	shape.init();
	
	GLSL::checkVersion();
}

void reshapeGL(int w, int h)
{
	// Set view size
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	float aspect = w/(float)h;
	camera.setPerspective(aspect, 45.0f/180.0f*M_PI, 1.0f, 1000.0f);
	HUD::setWidthHeight(w, h);
}

void drawGL()
{
	float tDispCurr = 0.001f*glutGet(GLUT_ELAPSED_TIME); // in seconds
	float dt = (tDispCurr - tDispPrev);
	float fps1 = 1.0f / dt;
	float a = 0.2f;
	fps = (1.0f - a) * fps + a * fps1;
	tDispPrev = tDispCurr;
	
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
	
	// Apply camera transforms
	MatrixStack P, MV;
	camera.applyProjectionMatrix(&P);
	camera.applyViewMatrix(&MV);
	
	// Draw grid
	progSimple.bind();
	glUniformMatrix4fv(progSimple.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progSimple.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	glLineWidth(2.0f);
	float x0 = -5.0f;
	float x1 = 5.0f;
	float y0 = -5.0f;
	float y1 = 5.0f;
	int gridSize = 10;
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	for(int i = 1; i < gridSize; ++i) {
		if(i == gridSize/2) {
			glColor3f(0.8f, 0.8f, 0.8f);
		} else {
			glColor3f(0.2f, 0.2f, 0.2f);
		}
		float x = x0 + i / (float)gridSize * (x1 - x0);
		glVertex2f(x, y0);
		glVertex2f(x, y1);
	}
	for(int i = 1; i < gridSize; ++i) {
		if(i == gridSize/2) {
			glColor3f(0.8f, 0.8f, 0.8f);
		} else {
			glColor3f(0.2f, 0.2f, 0.2f);
		}
		float y = y0 + i / (float)gridSize * (y1 - y0);
		glVertex2f(x0, y);
		glVertex2f(x1, y);
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(x0, y0, 0.0f);
	glVertex3f(x1, y0, 0.0f);
	glVertex3f(x1, y1, 0.0f);
	glVertex3f(x0, y1, 0.0f);
	glEnd();
	progSimple.unbind();
	
	// Draw particles
	prog.bind();
	glUniformMatrix4fv(prog.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	MV.pushMatrix();
	for(int i = 0; i < (int)particles.size(); ++i) {
		particles[i]->draw(MV, prog);
	}
	MV.popMatrix();
	prog.unbind();
	
	// Draw stats
	char str[256];
	glColor4f(0, 1, 0, 1);
	sprintf(str, "fps: %.1f", fps);
	HUD::drawString(10, HUD::getHeight()-15, str);
	sprintf(str, "t: %.2f   type: %d", stepper.t, stepper.type);
	HUD::drawString(10, 10, str);
	
	// Double buffer
	glutSwapBuffers();
}

void mouseGL(int button, int state, int x, int y)
{
	int modifiers = glutGetModifiers();
	bool shift = modifiers & GLUT_ACTIVE_SHIFT;
	bool ctrl  = modifiers & GLUT_ACTIVE_CTRL;
	bool alt   = modifiers & GLUT_ACTIVE_ALT;
	camera.mouseClicked(x, y, shift, ctrl, alt);
}

void mouseMotionGL(int x, int y)
{
	camera.mouseMoved(x, y);
}

void quit()
{
	// http://stackoverflow.com/questions/594089/does-stdvector-clear-do-delete-free-memory-on-each-element
	while(!particles.empty()) {
		delete particles.back();
		particles.pop_back();
	}
	exit(0);
}

void keyboardGL(unsigned char key, int x, int y)
{
	keyToggles[key] = !keyToggles[key];
	switch(key) {
		case 27:
			// ESCAPE
			quit();
			break;
		case 'h':
			stepper.step(particles);
			break;
		case 'r':
			stepper.reset(particles);
			break;
		case 't':
			stepper.type = (stepper.type + 1) % 3;
			break;
	}
}

void drawTimerGL(int value)
{
	glutPostRedisplay();
	glutTimerFunc(20, drawTimerGL, 0);
}

void simTimerGL(int value)
{
	if(keyToggles[' ']) {
		stepper.step(particles);
	}
	glutTimerFunc(5, simTimerGL, 0);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(400, 400);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Your Name");
	glutMouseFunc(mouseGL);
	glutMotionFunc(mouseMotionGL);
	glutKeyboardFunc(keyboardGL);
	glutReshapeFunc(reshapeGL);
	glutDisplayFunc(drawGL);
	glutTimerFunc(100, drawTimerGL, 0);
	glutTimerFunc(100, simTimerGL, 0);
	loadScene();
	initGL();
	glutMainLoop();
	return 0;
}
