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
#include "Particle.h"
#include "Texture.h"
#include "Obstacle.h"

using namespace std;

bool keyToggles[256] = {false};

float t = 0.0f;
float tPrev = 0.0f;
float h = 0.01f;
int width = 1;
int height = 1;
bool inSession = false;
bool gameOver = false;

Program prog;
Program progTex;
Program progParticle;
Camera camera;
Shape rocket;
ShapeObj skySphere;
Texture skySphereTex;
Texture sunTex;
Texture earthTex;
Texture particleTex;
vector<Particle *> particles;
vector<Obstacle *> obstacles;
Eigen::Vector3f light = Eigen::Vector3f(0, 5, -4);

void loadScene() {
	t = 0.0f;
	h = 0.01f;

	rocket.addObj("../models/roket.obj", "../models", Eigen::Vector3f(0,0,0), Eigen::Vector3f(0,0,1));

	skySphere.load("../models/sphere2.obj");
	skySphereTex.setFilename("../models/universe-photo.jpg");
	
	sunTex.setFilename("../models/sun.jpg");
	earthTex.setFilename("../models/earth.jpg");

	int nObs = 8;
	for (int i = 0; i < nObs; i++) {
		Obstacle *o = new Obstacle("../models/sphere2.obj");
		obstacles.push_back(o);
	}
	// Set obstacle positions
	obstacles[0]->moveTo(Eigen::Vector3f(0,  -1,  -1));
	obstacles[1]->moveTo(Eigen::Vector3f(1,   0.5, 2));
	obstacles[2]->moveTo(Eigen::Vector3f(1.5, 2.5, 1));
	obstacles[3]->moveTo(Eigen::Vector3f(3,   5,   1));
	obstacles[4]->moveTo(Eigen::Vector3f(5,   2,  -3));
	obstacles[5]->moveTo(Eigen::Vector3f(5,   4,  -2));
	obstacles[6]->moveTo(Eigen::Vector3f(7,   6,  0));
	obstacles[7]->moveTo(Eigen::Vector3f(10,   10,  0));

	particleTex.setFilename("../models/alpha.jpg");

	prog.setShaderNames("simple_vert.glsl", "simple_frag.glsl");
	progTex.setShaderNames("tex_vert.glsl", "tex_frag.glsl");
	progParticle.setShaderNames("particle_vert.glsl", "particle_frag.glsl");

	int n = 250;
	for(int i = 0; i < n; ++i) {
		Particle *p = new Particle();
		particles.push_back(p);
		p->setProgram(&progParticle);
		p->setKeyToggles(keyToggles);
		p->attachTo(&rocket);
		p->load();
	}
}

void initGL() {
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
	// Intialize the shapes
	//////////////////////////////////////////////////////
	
	skySphere.init();
	skySphereTex.init();
	sunTex.init();
	earthTex.init();
	particleTex.init();
	for (int i = 0; i < obstacles.size(); i++) {
		obstacles[i]->init();
	}
	rocket.init();
	// Add ControlBoxes (Position, Dimensions, first/last)
	rocket.addCB(Eigen::Vector3f(0,-1,0), Eigen::Vector3f(1,1,5), true);
	rocket.addCB(Eigen::Vector3f(1,0,3), Eigen::Vector3f(1,1,3), false);
	rocket.addCB(Eigen::Vector3f(1,2,2), Eigen::Vector3f(1,1,2), false);
	rocket.addCB(Eigen::Vector3f(2,0,0), Eigen::Vector3f(1,1,4), false);
	rocket.addCB(Eigen::Vector3f(2,2,1), Eigen::Vector3f(1,3,1), false);
	rocket.addCB(Eigen::Vector3f(4,5,0), Eigen::Vector3f(2,1,2), false);
	rocket.addCB(Eigen::Vector3f(4,3,-3), Eigen::Vector3f(1,1,3), false);
	rocket.addCB(Eigen::Vector3f(5,2,0), Eigen::Vector3f(1,1,1), false);
	rocket.addCB(Eigen::Vector3f(6,6,0), Eigen::Vector3f(1,2,1), false);
	rocket.addCB(Eigen::Vector3f(10,10,0), Eigen::Vector3f(0,0,0), true);

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

	progParticle.init();
	progParticle.addUniform("P");
	progParticle.addUniform("MV");
	progParticle.addAttribute("vertPos");
	progParticle.addAttribute("vertTex");
	progParticle.addUniform("radius");
	progParticle.addUniform("alphaTexture");
	progParticle.addUniform("color");
	
	for(int i = 0; i < (int)particles.size(); ++i) {
		particles[i]->init();
	}
	
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

// Sort particles by their z values in camera space
class ParticleSorter {
public:
	bool operator()(const Particle *p0, const Particle *p1) const
	{
		// Particle positions in world space
		const Eigen::Vector3f &x0 = p0->getPosition();
		const Eigen::Vector3f &x1 = p1->getPosition();
		// Particle positions in camera space
		float z0 = V.row(2) * Eigen::Vector4f(x0(0), x0(1), x0(2), 1.0f);
		float z1 = V.row(2) * Eigen::Vector4f(x1(0), x1(1), x1(2), 1.0f);
		return z0 < z1;
	}
	
	Eigen::Matrix4f V; // current camera matrix
};
ParticleSorter sorter;

void checkCollisions() {
	for (int i = 0; i < obstacles.size(); i++) {
 		if ((rocket.pos - obstacles[i]->pos).norm() < obstacles[i]->radius) {
 			inSession = false;
 			gameOver = true;
 		}
	}
}

void drawGL() {
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	//sky sphere
	skySphereTex.bind(progTex.getUniform("colorTexture"), 0);
	MV.pushMatrix();
	MV.scale(120);
	glUniformMatrix4fv(progTex.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progTex.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	skySphere.draw(progTex.getAttribute("vertPos"), -1, progTex.getAttribute("vertTex"));
	MV.popMatrix();
	skySphereTex.unbind();
	//suns
	sunTex.bind(progTex.getUniform("colorTexture"), 0);
	glUniformMatrix4fv(progTex.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progTex.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	for (int i = 0; i < obstacles.size()-1; i++) {
		obstacles[i]->draw(progTex, MV);
	}
	sunTex.unbind();
	// earth
	earthTex.bind(progTex.getUniform("colorTexture"), 0);
	glUniformMatrix4fv(progTex.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	glUniformMatrix4fv(progTex.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	obstacles[obstacles.size()-1]->draw(progTex, MV);
	earthTex.unbind();
	progTex.unbind();

	if (inSession) {
		checkCollisions();
	}
	// Sort the particles by Z
	sorter.V = MV.topMatrix();
	sort(particles.begin(), particles.end(), sorter);
	
	// Draw particles
	progParticle.bind();
	particleTex.bind(progParticle.getUniform("alphaTexture"), 0);
	glUniformMatrix4fv(progParticle.getUniform("P"), 1, GL_FALSE, P.topMatrix().data());
	MV.pushMatrix();
	for(int i = 0; i < (int)particles.size(); ++i) {
		particles[i]->draw(&MV);
	}
	MV.popMatrix();
	particleTex.unbind();
	progParticle.unbind();

	

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
		case 'p':
			if (!gameOver) {
				inSession = !inSession;
			}
			break;
		case ' ': // begin
			if (!gameOver) {
				inSession = true;
				for(int i = 0; i < (int)particles.size(); ++i) {
					particles[i]->reset();
				}
			}
			break;
		case 'r': // restart
			t = 0.0f;
			inSession = false;
			gameOver = false;
			for(int i = 0; i < (int)particles.size(); ++i) {
				particles[i]->reset();
				particles[i]->update(t, h);
			}
			break;
		case '.': // next ControlBox
			rocket.switchCB(1);
			break;
		case ',': // prev ControlBox
			rocket.switchCB(-1);
			break;
		case 'b':
			if(!inSession && !gameOver) {
				rocket.xMove(0.1);
			}
			break;
		case 'v':
			if(!inSession && !gameOver) {
				rocket.xMove(-0.1);
			}
			break;
		case 'u':
			if(!inSession && !gameOver) {
				rocket.yMove(0.1);
			}
			break;
		case 'j':
			if(!inSession && !gameOver) {
				rocket.yMove(-0.1);
			}
			break;
		case 'm':
			if(!inSession && !gameOver) {
				rocket.zMove(0.1);
			}
			break;
		case 'n':
			if(!inSession && !gameOver) {
				rocket.zMove(-0.1);
			}
			break;
	}
}

void timerGL(int value) {
	// Elapsed time
	float tCurr = 0.001f*glutGet(GLUT_ELAPSED_TIME); // in seconds
	float dt = (tCurr - tPrev);
	if (inSession) {
		for(int i = 0; i < (int)particles.size(); ++i) {
			particles[i]->update(t, h);
		}
		t += dt;
	}
	tPrev = tCurr;
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
