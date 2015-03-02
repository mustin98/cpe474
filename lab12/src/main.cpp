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
#include "Scene.h"
#include "HUD.h"

using namespace std;

int width = 1;
int height = 1;
bool keyToggles[256] = {false};
Camera camera;
Program prog;
Program progSimple;
Scene *scene;
float tDisp;
float tDispPrev;
float fps;
float tSim;
float sps;

void loadScene()
{
	tDisp = 0.0f;
	tDispPrev = 0.0f;
	fps = 0.0f;
	tSim = 0.0f;
	sps = 0.0f;
	keyToggles['c'] = false;

	camera.setTranslations(Eigen::Vector3f(0.0f, 0.0f, 1.5f));
	progSimple.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
	prog.setShaderNames("phong_vert.glsl", "phong_frag.glsl");
	prog.setVerbose(true);
	
	scene = new Scene();
	scene->load();
	scene->tare();
}

void initGL()
{
	// Set background color
	glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
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
	
	scene->init();
	
	GLSL::checkVersion();
}

void reshapeGL(int w, int h)
{
	// Set view size
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	float aspect = w/(float)h;
	camera.setPerspective(aspect, 45.0f/180.0f*M_PI, 0.01f, 100.0f);
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
	float x0 = -0.5f;
	float x1 = 0.5f;
	float z0 = -0.5f;
	float z1 = 0.5f;
	int gridSize = 10;
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	for(int i = 1; i < gridSize; ++i) {
		if(i == gridSize/2) {
			glColor3f(0.8f, 0.8f, 0.8f);
		} else {
			glColor3f(0.4f, 0.4f, 0.4f);
		}
		float x = x0 + i / (float)gridSize * (x1 - x0);
		glVertex3f(x, 0.0f, z0);
		glVertex3f(x, 0.0f, z1);
	}
	for(int i = 1; i < gridSize; ++i) {
		if(i == gridSize/2) {
			glColor3f(0.8f, 0.8f, 0.8f);
		} else {
			glColor3f(0.4f, 0.4f, 0.4f);
		}
		float z = z0 + i / (float)gridSize * (z1 - z0);
		glVertex3f(x0, 0.0f, z);
		glVertex3f(x1, 0.0f, z);
	}
	glEnd();
	glColor3f(0.4f, 0.4f, 0.4f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(x0, 0.0f, z0);
	glVertex3f(x1, 0.0f, z0);
	glVertex3f(x1, 0.0f, z1);
	glVertex3f(x0, 0.0f, z1);
	glEnd();
	progSimple.unbind();
	
	// Draw scene
	prog.bind();
	glUniformMatrix4fv(prog.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	MV.pushMatrix();
	scene->draw(MV, prog);
	MV.popMatrix();
	prog.unbind();
	
	// Draw stats
	char str[256];
	glColor4f(0, 1, 0, 1);
	sprintf(str, "fps: %.1f", fps);
	HUD::drawString(10, HUD::getHeight()-15, str);
	sprintf(str, "t: %.2f s   Each step takes %.1f ms", scene->getTime(), sps);
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
	delete scene;
	exit(0);
}

void step()
{
	float tSim0 = glutGet(GLUT_ELAPSED_TIME); // in ms
	
	scene->step();

	float tSim1 = glutGet(GLUT_ELAPSED_TIME); // in ms
	float dt = (tSim1 - tSim0);
	float sps1 = dt;
	float a = 0.2f;
	sps = (1.0f - a) * sps + a * sps1;
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
			step();
			break;
		case 'r':
			scene->reset();
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
		step();
	}
	glutTimerFunc(1, simTimerGL, 0);
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
