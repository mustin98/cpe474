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
#include <iostream>
#include "Particle.h"
#include "Shape.h"
#include "Program.h"
#include "MatrixStack.h"

using namespace std;

Particle::Particle(Shape *s) :
	r(1.0),
	m(1.0),
	x(0.0, 0.0, 0.0),
	v(0.0, 0.0, 0.0),
	shape(s)
{
	
}

Particle::~Particle()
{
	
}

void Particle::tare()
{
	x0 = x;
	v0 = v;
}

void Particle::reset()
{
	x = x0;
	v = v0;
}

void Particle::draw(MatrixStack &MV, const Program &prog) const
{
	MV.pushMatrix();
	MV.translate(Eigen::Vector3f(x(0), x(1), x(2)));
	MV.scale(r);
	glUniformMatrix4fv(prog.getUniform("MV"), 1, GL_FALSE, MV.topMatrix().data());
	shape->draw(prog);
	MV.popMatrix();
}
