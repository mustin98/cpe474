#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <iostream>
#include <vector>
#include <memory>
#include "Program.h"
#include "GLSL.h"
#include <Eigen/Dense>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

bool keyToggles[256] = {false};
bool rotating = false;
Eigen::Vector2f mousePrev;
Eigen::Vector2f cameraRotations;

float t = 0.0f;
float h = 1.1f;

Program prog;
int width;
int height;

// Display time to control fps
float t0_disp = 0.0f;
float t_disp = 0.0f;

// Control points
vector<Eigen::Vector3f> cps;

enum SplineType
{
	BEZIER = 0,
	CATMULL_ROM,
	BASIS,
	SPLINE_TYPE_COUNT
};

SplineType type = BEZIER;

void loadScene()
{
	t = 0.0f;
	h = 0.001f;
	mousePrev(0) = 0.0f;
	mousePrev(1) = 0.0f;
	cameraRotations(0) = 0.0f;
	cameraRotations(1) = 0.0f;
	
	prog.setShaderNames("lab02_vert.glsl", "lab02_frag.glsl");
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
	// Set view size
	width = w;
	height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

Eigen::Matrix4f getOrtho2D(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Eigen::Matrix4f M = Eigen::Matrix4f::Zero();
	M(0,0) = 2.0f / (right - left);
	M(1,1) = 2.0f / (top - bottom);
	M(2,2) = -2.0f / (zFar - zNear);
	M(0,3) = - (right + left) / (right - left);
	M(1,3) = - (top + bottom) / (top - bottom);
	M(2,3) = - (zFar + zNear) / (zFar - zNear);
	M(3,3) = 1.0f;
	return M;
}

Eigen::Matrix4f getRotateX(float radians)
{
	Eigen::Matrix4f M = Eigen::Matrix4f::Identity();
	float s = std::sin(radians);
	float c = std::cos(radians);
	M(1,1) = c;
	M(1,2) = -s;
	M(2,1) = s;
	M(2,2) = c;
	return M;
}

Eigen::Matrix4f getRotateY(float radians)
{
	Eigen::Matrix4f M = Eigen::Matrix4f::Identity();
	float s = std::sin(radians);
	float c = std::cos(radians);
	M(0,0) = c;
	M(0,2) = s;
	M(2,0) = -s;
	M(2,2) = c;
	return M;
}


Eigen::Matrix4f getBezierMatrix() {
	Eigen::Matrix4f B;
	B(0,0) = 1;
	B(0,1) = -3;
	B(0,2) = 3;
	B(0,3) = -1;

	B(1,0) = 0;
	B(1,1) = 3;
	B(1,2) = -6;
	B(1,3) = 3;
	
	B(2,0) = 0;
	B(2,1) = 0;
	B(2,2) = 3;
	B(2,3) = -3;
	
	B(3,0) = 0;
	B(3,1) = 0;
	B(3,2) = 0;
	B(3,3) = 1;
	return B;
}

Eigen::Matrix4f getCatmullMatrix() {
	Eigen::Matrix4f B;
	B(0,0) = 0;
	B(0,1) = -1;
	B(0,2) = 2;
	B(0,3) = -1;

	B(1,0) = 2;
	B(1,1) = 0;
	B(1,2) = -5;
	B(1,3) = 3;
	
	B(2,0) = 0;
	B(2,1) = 1;
	B(2,2) = 4;
	B(2,3) = -3;
	
	B(3,0) = 0;
	B(3,1) = 0;
	B(3,2) = -1;
	B(3,3) = 1;
	return B * 0.5;
}

Eigen::Matrix4f getBSplineMatrix() {
	Eigen::Matrix4f B;
	B(0,0) = 1;
	B(0,1) = -3;
	B(0,2) = 3;
	B(0,3) = -1;

	B(1,0) = 4;
	B(1,1) = 0;
	B(1,2) = -6;
	B(1,3) = 3;
	
	B(2,0) = 1;
	B(2,1) = 3;
	B(2,2) = 3;
	B(2,3) = -3;
	
	B(3,0) = 0;
	B(3,1) = 0;
	B(3,2) = 0;
	B(3,3) = 1;
	return B * 1.0 / 6.0;
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
	
	Eigen::Matrix4f P = getOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 2.0f);
	Eigen::Matrix4f MV = getRotateY(cameraRotations(0)) * getRotateX(cameraRotations(1));
	
	// Bind the program
	prog.bind();
	glUniformMatrix4fv(prog.getUniform("P"), 1, GL_TRUE, P.data());
	glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_TRUE, MV.data());
	
	// Draw control points
	int ncps = (int)cps.size();
	glPointSize(3.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for(int i = 0; i < ncps; ++i) {
		Eigen::Vector3f cp = cps[i];
		glVertex3f(cp(0), cp(1), cp(2));
	}
	glEnd();	
	// 4 by 4 matrix
	Eigen::Vector4f uVec;
	Eigen::Vector4f uVec1;
	Eigen::Vector4f uVec2;
	// 4 by 1 vector
	
	// Draw polyline
	glLineWidth(1.0f);
	if(keyToggles['l'] &&  ncps >= 4) {
		// Fill in G, B, and uVec
		Eigen::Matrix4f B;
		Eigen::MatrixXf G(3,4);
		if (type == BEZIER) {
			B = getBezierMatrix();
			// 3 by 4 matrix
			for (int i = 0; i < 4 && i < ncps; i++) {
				G(0,i) = cps[i](0);
				G(1,i) = cps[i](1);
				G(2,i) = cps[i](2);
			}//Only first 4 pts
			glBegin(GL_LINE_STRIP);
			glColor3f(1.0f, 0.0f, 1.0f);
			for (float u = 0; u < 1; u += 0.001) {
				uVec(0) = 1;
				uVec(1) = u;
				uVec(2) = u*u;
				uVec(3) = u*u*u;
				Eigen::Vector3f p = G*B*uVec;
				// 3 by 1 vector				
				glVertex3fv(p.data());				
			}
			glEnd();
		}
		else if (type == CATMULL_ROM) {
			B = getCatmullMatrix();
			for (int i = 0; i <= ncps - 4 && ncps >= 4; i++) {
				for (int idx = i; idx < i + 4; idx++) {
					G(0,idx - i) = cps[idx](0);
					G(1,idx - i) = cps[idx](1);
					G(2,idx - i) = cps[idx](2);
				}		
				glBegin(GL_LINE_STRIP);
				glColor3f(1.0f, 0.0f, 1.0f);
				for (float u = 0; u < 1; u += 0.01) {
					uVec(0) = 1;
					uVec(1) = u;
					uVec(2) = u*u;
					uVec(3) = u*u*u;
					Eigen::Vector3f p = G*B*uVec;
					
					// 3 by 1 vector
					glVertex3fv(p.data());
				}
				glEnd();
			}
			float kfloat;
			float u = std::modf(std::fmod(t*0.01f, ncps - 3.0f), &kfloat);
			int k = (int)std::floor(kfloat);

			uVec(0) = 1;
			uVec(1) = u;
			uVec(2) = u*u;
			uVec(3) = u*u*u;
			for (int idx = k; idx < k + 4; idx++) {
				G(0, idx - k) = cps[idx](0);
				G(1, idx - k) = cps[idx](1);
				G(2, idx - k) = cps[idx](2);
			}
			Eigen::Vector3f p = G*B*uVec;
			uVec1(0) = 0;
			uVec1(1) = 1;
			uVec1(2) = 2*u;
			uVec1(3) = 3*u*u;
			Eigen::Vector3f p1 = G*B*uVec1;
			Eigen::Vector3f p1norm = p1;
			p1norm.normalize();
			uVec2(0) = 0;
			uVec2(1) = 0;
			uVec2(2) = 2;
			uVec2(3) = 6*u;
			Eigen::Vector3f p2 = G*B*uVec2;
			Eigen::Vector3f p2norm = p2;
			p2norm.normalize();
			Eigen::Vector3f p1xp2 = p1.cross(p2);
			p1xp2.normalize();

			Eigen::Vector3f T = p1norm;
			Eigen::Vector3f BiNorm = p1xp2;
			Eigen::Vector3f N = BiNorm.cross(T);
			T *= .15;
			BiNorm *= .15;
			N *= .15;

			glBegin(GL_LINE_STRIP);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3fv(p.data());
			glVertex3f((p+T)(0), (p+T)(1), (p+T)(2));
			glEnd();

			glBegin(GL_LINE_STRIP);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3fv(p.data());
			glVertex3f((p+N)(0), (p+N)(1), (p+N)(2));
			glEnd();

			glBegin(GL_LINE_STRIP);
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3fv(p.data());
			glVertex3f((p+BiNorm)(0), (p+BiNorm)(1), (p+BiNorm)(2));
			glEnd();
		}
		else if (type == BASIS) {
			B = getBSplineMatrix();
			for (int i = 0; i <= ncps - 4 && ncps >= 4; i++) {
				for (int idx = i; idx < i + 4; idx++) {
					G(0,idx - i) = cps[idx](0);
					G(1,idx - i) = cps[idx](1);
					G(2,idx - i) = cps[idx](2);
				}		
				glBegin(GL_LINE_STRIP);
				glColor3f(1.0f, 0.0f, 1.0f);
				for (float u = 0; u < 1; u += 0.01) {
					uVec(0) = 1;
					uVec(1) = u;
					uVec(2) = u*u;
					uVec(3) = u*u*u;
					Eigen::Vector3f p = G*B*uVec;
					// 3 by 1 vector				
					glVertex3fv(p.data());				
				}
				glEnd();
			}

			float kfloat;
			float u = std::modf(std::fmod(t*0.01f, ncps - 3.0f), &kfloat);
			int k = (int)std::floor(kfloat);

			uVec(0) = 1;
			uVec(1) = u;
			uVec(2) = u*u;
			uVec(3) = u*u*u;
			for (int idx = k; idx < k + 4; idx++) {
				G(0, idx - k) = cps[idx](0);
				G(1, idx - k) = cps[idx](1);
				G(2, idx - k) = cps[idx](2);
			}
			Eigen::Vector3f p = G*B*uVec;
			uVec1(0) = 0;
			uVec1(1) = 1;
			uVec1(2) = 2*u;
			uVec1(3) = 3*u*u;
			Eigen::Vector3f p1 = G*B*uVec1;
			Eigen::Vector3f p1norm = p1;
			p1norm.normalize();
			uVec2(0) = 0;
			uVec2(1) = 0;
			uVec2(2) = 2;
			uVec2(3) = 6*u;
			Eigen::Vector3f p2 = G*B*uVec2;
			Eigen::Vector3f p2norm = p2;
			p2norm.normalize();
			Eigen::Vector3f p1xp2 = p1.cross(p2);
			p1xp2.normalize();

			Eigen::Vector3f T = p1norm;
			Eigen::Vector3f BiNorm = p1xp2;
			Eigen::Vector3f N = BiNorm.cross(T);
			T *= .25;
			BiNorm *= .25;
			N *= .25;

			glBegin(GL_LINE_STRIP);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3fv(p.data());
			glVertex3f((p+T)(0), (p+T)(1), (p+T)(2));
			glEnd();

			glBegin(GL_LINE_STRIP);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3fv(p.data());
			glVertex3f((p+N)(0), (p+N)(1), (p+N)(2));
			glEnd();

			glBegin(GL_LINE_STRIP);
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3fv(p.data());
			glVertex3f((p+BiNorm)(0), (p+BiNorm)(1), (p+BiNorm)(2));
			glEnd();
		}
		glColor3f(1.0f, 0.5f, 0.5f);
		glBegin(GL_LINE_STRIP);
		for(int i = 0; i < ncps; ++i) {
			Eigen::Vector3f cp = cps[i];
			// You can also specify an array by using glVertex3fv()
			glVertex3fv(cp.data());
		}
		glEnd();
	}

	// Unbind the program
	prog.unbind();

	// Double buffer
	glutSwapBuffers();
}

void mouseGL(int button, int state, int x_, int y_)
{
	int modifier = glutGetModifiers();
	bool shift = modifier & GLUT_ACTIVE_SHIFT;
	bool ctrl  = modifier & GLUT_ACTIVE_CTRL;
	bool alt   = modifier & GLUT_ACTIVE_ALT;
	
	rotating = false;
	if(!shift && !ctrl && !alt) {
		rotating = true;
		mousePrev(0) = x_;
		mousePrev(1) = y_;
	}
	
	// Add a new control point
	if(state == GLUT_DOWN && shift) {
		// Convert from window coord to world coord
		Eigen::Vector4f x;
		x(0) = x_ / (float)width;
		x(1) = (height - y_) / (float)height;
		x(0) = 2.0f * (x(0) - 0.5f);
		x(1) = 2.0f * (x(1) - 0.5f);
		x(2) = 0.0;
		x(3) = 1.0;
		// Apply current rotation matrix
		Eigen::Matrix4f R = getRotateY(cameraRotations(0)) * getRotateX(cameraRotations(1));
		x = R * x;
		cps.push_back(x.segment<3>(0));
	}
}

void mouseMotionGL(int x_, int y_)
{
	if(rotating) {
		Eigen::Vector2f mouse(x_, y_);
		cameraRotations += 0.01f * (mouse - mousePrev);
		mousePrev = mouse;
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
		case 's':
			type = (SplineType)((type + 1) % SPLINE_TYPE_COUNT);
			break;
		case 'c':
			cps.clear();
			break;
	}
}

void idleGL()
{
	if(keyToggles[' ']) {
		t += h;
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
	glutCreateWindow("Todd & Justin");
	glutMouseFunc(mouseGL);
	glutMotionFunc(mouseMotionGL);
	glutKeyboardFunc(keyboardGL);
	glutReshapeFunc(reshapeGL);
	glutDisplayFunc(drawGL);
	glutIdleFunc(idleGL);
	loadScene();
	initGL();
	glutMainLoop();
	return 0;
}
