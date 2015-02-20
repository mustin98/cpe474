#pragma once
#ifndef _PARTICLE_H_
#define _PARTICLE_H_

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
#include <vector>
#include <Eigen/Dense>

class MatrixStack;
class Program;
class Texture;

class Particle
{
public:
	Particle();
	virtual ~Particle();
	void load();
	void setProgram(Program *p) { prog = p; }
	void setKeyToggles(const bool *k) { keyToggles = k; }
	void init();
	void draw(MatrixStack *MV) const;
	void rebirth(float t);
	void update(float t, float h);
	const Eigen::Vector3f& getPosition() const { return x; };
	const Eigen::Vector3f& getVelocity() const { return v; };
	
private:
	float m; // mass
	float d; // viscous damping
	Eigen::Vector3f x; // position
	Eigen::Vector3f v; // velocity
	float lifespan; // how long this particle lives
	float tEnd;     // time this particle dies
	float radius;
	Eigen::Vector4f color;
	std::vector<float> posBuf;
	std::vector<float> texBuf;
	std::vector<unsigned int> indBuf;
	GLuint posBufID;
	GLuint texBufID;
	GLuint indBufID;
	Program *prog;
	const bool *keyToggles;
};

#endif
