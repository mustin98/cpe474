#pragma once
#ifndef __L12__Cloth__
#define __L12__Cloth__

#include <stdio.h>
#include <vector>
#include <Eigen/Dense>

class Particle;
class Spring;
class Shape;
class MatrixStack;
class Program;

class Cloth
{
public:
	Cloth(const Shape *sphere,
		  int rows, int cols,
		  const Eigen::Vector3d &x00,
		  const Eigen::Vector3d &x01,
		  const Eigen::Vector3d &x10,
		  const Eigen::Vector3d &x11,
		  double mass, double stiffness);
	virtual ~Cloth();
	
	void tare();
	void reset();
	void updatePosNor();
	void step(double h, const Eigen::Vector3d &grav);
	
	void init();
	void draw(MatrixStack &MV, const Program &p) const;
	
	void setImplicit(bool i) { imp = i; }
	void setShowParticles(bool s) { showParticles = s; }
	
private:
	int rows;
	int cols;
	int n;
	std::vector<Particle*> particles;
	std::vector<Spring*> springs;
	bool imp;
	
	Eigen::VectorXd v;
	Eigen::VectorXd f;
	Eigen::MatrixXd M;
	Eigen::MatrixXd K;
	
	bool showParticles;
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
};

#endif /* defined(__L12__Cloth__) */
