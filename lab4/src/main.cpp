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

using namespace std;

bool keyToggles[256] = {false};
bool rotating = false;
Eigen::Vector2f mousePrev;
Eigen::Vector2f cameraRotations;

float t = 0.0f;
float tPrev = 0.0f;

Program prog;
int width = 1;
int height = 1;

// Control points
vector<Eigen::Vector3f> cps;

enum SplineType
{
	CATMULL_ROM = 0,
	BASIS,
	SPLINE_TYPE_COUNT
};

SplineType type = CATMULL_ROM;

Eigen::Matrix4f Bcr, Bb;

vector<pair<float,float> > usTable;

void loadScene()
{
	t = 0.0f;
	mousePrev(0) = 0.0f;
	mousePrev(1) = 0.0f;
	cameraRotations(0) = 0.0f;
	cameraRotations(1) = 0.0f;
	
	Bcr << 0.0f, -1.0f,  2.0f, -1.0f,
		   2.0f,  0.0f, -5.0f,  3.0f,
		   0.0f,  1.0f,  4.0f, -3.0f,
		   0.0f,  0.0f, -1.0f,  1.0f;
	Bcr *= 0.5;
	
	Bb << 1.0f, -3.0f,  3.0f, -1.0f,
		  4.0f,  0.0f, -6.0f,  3.0f,
		  1.0f,  3.0f,  3.0f, -3.0f,
		  0.0f,  0.0f,  0.0f,  1.0f;
	Bb /= 6.0f;
	
	prog.setShaderNames("lab03_vert.glsl", "lab03_frag.glsl");
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

Eigen::Vector4f getUVec(float u) {
	return Eigen::Vector4f(1, u, u*u, u*u*u);
}
Eigen::Vector4f getUVecPrime(float u) {
	return Eigen::Vector4f(0, 1, 2*u, 3*u*u);
}

void buildTable()
{
	usTable.clear();
	int ncps = (int)cps.size();
	Eigen::MatrixXf G(3,4);
	float totalLen = 0.0f;
	Eigen::Matrix4f B = (type == CATMULL_ROM ? Bcr : Bb);

	if(ncps >= 4) {
		//BUILD TABLE
		usTable.push_back(make_pair(0.0f, totalLen));
		for (int cp = 0; cp < ncps-3; cp++) {
			for (int idx = cp; idx < cp+4; idx++) {
				G(0, idx-cp) = cps[idx](0);
				G(1, idx-cp) = cps[idx](1);
				G(2, idx-cp) = cps[idx](2);
			}
			for (float u = 0.2f; u < 1.0001f; u += 0.2f) {
				float uA = u - 0.2, uB = u;
				Eigen::Vector4f uVecA = getUVec(uA);
				Eigen::Vector4f uVecB = getUVec(uB);
				Eigen::Vector3f pPrime;

				float dx = sqrt(3.0f/5.0f);
				float dw = 3.0f/9.0f;
				float x = -dx, w = 5.0f/9.0f;
				float sum = 0;
				for (int i = 0; i < 3; i++) {
					float pParam = (uB - uA) / 2 * x + (uA + uB) / 2;
					Eigen::Vector4f uVecPrime = getUVecPrime(pParam);
					pPrime = G*B*uVecPrime;
					sum += w * pPrime.norm();
					
					x += dx;
					w += dw;
					dw = -dw;
				}
				float s = (uB - uA) / 2 * sum;

				//Eigen::Vector3f p1 = G*B*uVecA;
				//Eigen::Vector3f p2 = G*B*uVecB;

				//totalLen += (p2-p1).norm();
				totalLen += s;
				usTable.push_back(make_pair(cp + u, totalLen));
			}
		}

		// Print out the table
		for(int i = 0; i < (int)usTable.size(); ++i) {
			pair<float,float> row = usTable[i];
			cout << row.first << ", " << row.second << endl;
		}
	}
}

float s2u(float s)
{
	pair<float, float> start = usTable[0], end = usTable[1];
	for (int i = 0; i < (int)usTable.size() - 1; i++) {
		pair<float, float> row1 = usTable[i];
		pair<float, float> row2 = usTable[i+1];
		if (s > row1.second && s < row2.second) {
			start = usTable[i];
			end = usTable[i+1];
			break;
		}
	}
	float a = (s - start.second) / (end.second - start.second);
	float u = (1 - a) * start.first + a * end.first;
	return u;
}

void drawGL()
{
	// Elapsed time
	float tCurr = 0.001f*glutGet(GLUT_ELAPSED_TIME); // in seconds
	if(keyToggles[' ']) {
		t += (tCurr - tPrev);
	}
	tPrev = tCurr;
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Projection and modelview matrices
	Eigen::Matrix4f P = getOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 2.0f);
	Eigen::Matrix4f MV = getRotateY(cameraRotations(0)) * getRotateX(cameraRotations(1));
	
	// Bind the program
	prog.bind();
	glUniformMatrix4fv(prog.getUniform("P"), 1, GL_TRUE, P.data());
	glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_TRUE, MV.data());
	
	// Draw control points
	int ncps = (int)cps.size();
	glPointSize(5.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for(int i = 0; i < ncps; ++i) {
		glVertex3fv(cps[i].data());
	}
	glEnd();

	// Connect the control points
	glLineWidth(1.0f);
	if(keyToggles['l']) {
		glColor3f(0.5f, 0.5f, 0.5f);
		glBegin(GL_LINE_STRIP);
		for(int i = 0; i < ncps; ++i) {
			glVertex3fv(cps[i].data());
		}
		glEnd();
	}
	
	if(ncps >= 4) {
		// Draw spline
		Eigen::MatrixXf G(3,ncps);
		Eigen::MatrixXf Gk(3,4);
		Eigen::Matrix4f B = (type == CATMULL_ROM ? Bcr : Bb);
		for(int i = 0; i < ncps; ++i) {
			G.block<3,1>(0,i) = cps[i];
		}
		glLineWidth(3.0f);
		for(int k = 0; k < ncps - 3; ++k) {
			int n = 32; // curve discretization
			// Gk is the 3x4 block starting at column k
			Gk = G.block<3,4>(0,k);
			glBegin(GL_LINE_STRIP);
			for(int i = 0; i < n; ++i) {
				if(i/(n/2) % 2 == 0) {
					// First half color
					glColor3f(0.0f, 1.0f, 0.0f);
				} else {
					// Second half color
					glColor3f(0.0f, 0.0f, 1.0f);
				}
				// u goes from 0 to 1 within this segment
				float u = i / (n - 1.0f);
				// Compute spline point at u
				Eigen::Vector4f uVec(1.0f, u, u*u, u*u*u);
				Eigen::Vector3f P = Gk * B * uVec;
				glVertex3fv(P.data());
			}
			glEnd();
		}
		
		// Draw equally spaced points on the spline curve
		if(keyToggles['a'] && !usTable.empty()) {
			float ds = 0.2;
			glColor3f(1.0f, 0.0f, 0.0f);
			glPointSize(10.0f);
			glBegin(GL_POINTS);
			float smax = usTable.back().second; // spline length
			for(float s = 0.0f; s < smax; s += ds) {
				// Convert from s to (concatenated) u
				float uu = s2u(s);
				// Convert from concatenated u to the usual u between 0 and 1.
				float kfloat;
				float u = std::modf(uu, &kfloat);
				// k is the index of the starting control point
				int k = (int)std::floor(kfloat);
				// Gk is the 3x4 block starting at column k
				Gk = G.block<3,4>(0,k);
				// Compute spline point at u
				Eigen::Vector4f uVec(1.0f, u, u*u, u*u*u);
				Eigen::Vector3f P = Gk * B * uVec;
				glVertex3fv(P.data());
			}
			glEnd();
		}
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
		x(2) = 0.0f;
		x(3) = 1.0f;
		// Apply current rotation matrix
		Eigen::Matrix4f R = getRotateY(cameraRotations(0)) * getRotateX(cameraRotations(1));
		x = R * x;
		cps.push_back(x.segment<3>(0));
		buildTable();
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
			buildTable();
			break;
		case 'c':
			cps.clear();
			buildTable();
			break;
		case 't':
			t = 0.0f;
			break;
		case 'r':
			cps.clear();
			int n = 8;
			for(int i = 0; i < n; ++i) {
				float alpha = i / (n - 1.0f);
				float angle = 2.0f * M_PI * alpha;
				float radius = cos(2.0f * angle);
				Eigen::Vector3f cp;
				cp(0) = radius * cos(angle);
				cp(1) = radius * sin(angle);
				cp(2) = (1.0f - alpha)*(-0.5) + alpha*0.5;
				cps.push_back(cp);
			}
			buildTable();
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
	glutCreateWindow("Get carried, kid");
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
