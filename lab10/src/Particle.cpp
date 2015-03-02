#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"

using namespace std;

float randFloat(float l, float h)
{
	float r = rand() / (float)RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle() :
	m(1.0f),
	d(0.0f),
	x(0.0f, 0.0f, 0.0f),
	v(0.0f, 0.0f, 0.0f),
	lifespan(1.0f),
	tEnd(0.0f),
	radius(1.0f),
	color(1.0f, 1.0f, 1.0f, 1.0f),
	posBufID(0),
	texBufID(0),
	indBufID(0)
{
}

Particle::~Particle()
{
}

void Particle::load()
{
	// Load geometry
	// 0
	posBuf.push_back(-1.0f);
	posBuf.push_back(-1.0f);
	posBuf.push_back(0.0f);
	texBuf.push_back(0.0f);
	texBuf.push_back(0.0f);
	// 1
	posBuf.push_back(1.0f);
	posBuf.push_back(-1.0f);
	posBuf.push_back(0.0f);
	texBuf.push_back(1.0f);
	texBuf.push_back(0.0f);
	// 2
	posBuf.push_back(-1.0f);
	posBuf.push_back(1.0f);
	posBuf.push_back(0.0f);
	texBuf.push_back(0.0f);
	texBuf.push_back(1.0f);
	// 3
	posBuf.push_back(1.0f);
	posBuf.push_back(1.0f);
	posBuf.push_back(0.0f);
	texBuf.push_back(1.0f);
	texBuf.push_back(1.0f);
	// indices
	indBuf.push_back(0);
	indBuf.push_back(1);
	indBuf.push_back(2);
	indBuf.push_back(3);
	
	// Random initialization
	rebirth(0.0f);
}

void Particle::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the texture coordinates array to the GPU
	glGenBuffers(1, &texBufID);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	
	// Send the index array to the GPU
	glGenBuffers(1, &indBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void Particle::draw(MatrixStack *MV) const
{
	// Enable and bind position array for drawing
	GLint h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Enable and bind texcoord array for drawing
	GLint h_tex = prog->getAttribute("vertTex");
	GLSL::enableVertexAttribArray(h_tex);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Bind index array for drawing
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufID);
	
	// Transformation matrix
	MV->pushMatrix();
	MV->translate(x);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
	MV->popMatrix();
	
	// Color and scale
	glUniform4fv(prog->getUniform("color"), 1, color.data());
	glUniform1f(prog->getUniform("radius"), radius);
	
	// Draw
	glDrawElements(GL_TRIANGLE_STRIP, (int)indBuf.size(), GL_UNSIGNED_INT, 0);
	
	// Disable and unbind
	GLSL::disableVertexAttribArray(h_tex);
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Particle::rebirth(float t)
{
	//
	// <INITIALIZATION>
	//
	
	if (keyToggles['g']) {
		x << randFloat(0.3f, 0.5f), randFloat(0.1f, 0.4f), randFloat(-0.1f, 0.1f);
		v << randFloat(-4.0f, -1.0f), randFloat(1.0f, 5.0f), randFloat(-2.5f, 2.5f);
	} else {
		x << randFloat(-0.1f, 0.1f), randFloat(-0.1f, 0.1f), randFloat(-0.1f, 0.1f);
		v << randFloat(-15.0f, 15.0f), randFloat(0.0f, 15.0f), randFloat(-15.0f, 15.0f);
	}
	radius = randFloat(0.1f, 0.2f);
	d = .05f;
	lifespan = randFloat(3.5f, 5.0f);
	color << randFloat(0.6f, 0.9f), randFloat(0.5f, 0.6f), randFloat(0.5f, 0.6f), 1.0f;
	//
	// </INITIALIZATION>
	//

	float density = 1.0f;
	float volume = 4.0f/3.0f*M_PI*radius*radius*radius;
	m = density*volume;
	tEnd = t + lifespan;
}

void Particle::update(float t, float h)
{
	if(t > tEnd) {
		rebirth(t);
	}

	//
	// <UPDATE>
	//

	Eigen::Vector3f f(0.0f, 0.0f, 0.0f);
	if (keyToggles['g']) {
		float r = x.norm();
		float C = 2.5f;
		if (r > 0.1) {
			f = -(C*m)/(r*r)*(x/r);
		}
	}
	else {
		f = Eigen::Vector3f(0.0f, -9.8, 0);
		f *= m;
	}
	f += -d * v;
	v += h/m * f;
	x += h*v;
	if (keyToggles['f']) {
		if (x.y() < 0.0) {
			x.y() = 0.0;
			v = Eigen::Vector3f(v.x(), -v.y(), v.z());
			x += h*v;
		}
	}

	//
	// </UPDATE>
	//

	// Color
	color(3) = (tEnd-t)/lifespan;
}
